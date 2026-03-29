#include"DCMeshBuilder.h" 

// ---------------------------------------------------------------------------- 
void ContourProcessEdge(DCNode* node[4], int dir, IndexBuffer& indexBuffer)
{
	int minSize = 1000000;		// arbitrary big number
	int minIndex = 0;
	int indices[4] = { -1, -1, -1, -1 };
	bool flip = false;
	bool signChange[4] = { false, false, false, false };

	for (int i = 0; i < 4; i++)
	{
		const int edge = processEdgeMask[dir][i];
		const int c1 = edgevmap[edge][0];
		const int c2 = edgevmap[edge][1];

		const int m1 = (node[i]->drawInfo->corners >> c1) & 1;
		const int m2 = (node[i]->drawInfo->corners >> c2) & 1;

		if (node[i]->size < minSize)
		{
			minSize = node[i]->size;
			minIndex = i;
			flip = m1 != MATERIAL_AIR;
		}

		indices[i] = node[i]->drawInfo->index;

		signChange[i] =
			(m1 == MATERIAL_AIR && m2 != MATERIAL_AIR) ||
			(m1 != MATERIAL_AIR && m2 == MATERIAL_AIR);
	}

	if (signChange[minIndex])
	{
		if (!flip)
		{
			indexBuffer.push_back(indices[0]);
			indexBuffer.push_back(indices[1]);
			indexBuffer.push_back(indices[3]);

			indexBuffer.push_back(indices[0]);
			indexBuffer.push_back(indices[3]);
			indexBuffer.push_back(indices[2]);
		}
		else
		{
			indexBuffer.push_back(indices[0]);
			indexBuffer.push_back(indices[3]);
			indexBuffer.push_back(indices[1]);

			indexBuffer.push_back(indices[0]);
			indexBuffer.push_back(indices[2]);
			indexBuffer.push_back(indices[3]);
		}
	}
}

// ----------------------------------------------------------------------------

// ContourEdgeProc resolve UMA ARESTA compartilhada por 4 células
// Ele garante que todas estejam no MESMO nível de detalhe
// Se não estiverem, ele subdivide a aresta recursivamente
void ContourEdgeProc(DCNode* node[4], int dir, IndexBuffer& indexBuffer)
{
	// Se qualquer uma das 4 células não existe,
	// essa aresta não é válida
	if (!node[0] || !node[1] || !node[2] || !node[3])
	{
		return;
	}

	// ------------------------------------------------------------------
	// CASO BASE:
	// Se NENHUMA das 4 células é interna,
	// então todas estão no nível mais fino relevante
	// → podemos gerar triângulos com segurança
	if (node[0]->type != Node_Internal &&
		node[1]->type != Node_Internal &&
		node[2]->type != Node_Internal &&
		node[3]->type != Node_Internal)
	{
		// Aqui acontece a geração real de triângulos
		ContourProcessEdge(node, dir, indexBuffer);
	}
	else
	{
		// ------------------------------------------------------------------
		// CASO RECURSIVO:
		// Pelo menos uma das células é interna
		// Isso significa que a aresta real está "escondida"
		// dentro de subdivisões menores
		//
		// Uma aresta 3D, ao descer um nível da octree,
		// se divide em DUAS sub-arestas
		for (int i = 0; i < 2; i++)
		{
			DCNode* edgeNodes[4];

			// Para essa sub-aresta i:
			// quais filhos de cada célula participam? 
			const int c[4] =
			{
				edgeProcEdgeMask[dir][i][0],
				edgeProcEdgeMask[dir][i][1],
				edgeProcEdgeMask[dir][i][2],
				edgeProcEdgeMask[dir][i][3],
			};

			// Para cada uma das 4 células ao redor da aresta:
			for (int j = 0; j < 4; j++)
			{
				// Se a célula já é folha (ou pseudo-leaf),
				// ela já está no nível mais fino possível
				if (node[j]->type == Node_Leaf ||
					node[j]->type == Node_Psuedo)
				{
					edgeNodes[j] = node[j];
				}
				else
				{
					// Caso contrário, descemos para o filho
					// que toca essa sub-aresta específica
					edgeNodes[j] = node[j]->children[c[j]];
				}
			}

			// Recursão:
			// Continua descendo até que TODAS as 4 células
			// estejam no mesmo nível local
			ContourEdgeProc(edgeNodes, edgeProcEdgeMask[dir][i][4], indexBuffer);
		}
	}
}


// ContourFaceProc resolve diferenças de LOD ENTRE DUAS CÉLULAS que compartilham UMA FACE
// Ela NÃO gera triângulos diretamente (exceto via EdgeProc)
// Ela garante que TODAS as arestas internas dessa face sejam processadas no nível correto
void ContourFaceProc(DCNode* node[2], int dir, IndexBuffer& indexBuffer)
{
	// Se uma das células não existe, não há face válida
	if (!node[0] || !node[1])
		return;

	// Se ambas são folhas, essa face NÃO precisa ser subdividida
	// As arestas dela serão resolvidas diretamente no EdgeProc
	if (node[0]->type == Node_Internal || node[1]->type == Node_Internal)
	{
		// ------------------------------------------------------------------
		// ETAPA 1: SUBDIVIDIR A FACE EM 4 SUBFACES (caso exista LOD diferente)
		//
		// Uma face 3D, ao descer um nível na octree, vira 4 subfaces 2D
		// Precisamos tratar cada uma separadamente
		for (int i = 0; i < 4; i++)
		{
			DCNode* faceNodes[2];

			// Para a subface i e direção dir:
			// quais filhos do node[0] e node[1] realmente se tocam nessa subface?
			const int c[2] = {
				//Para essa subface i e direção dir, quais filhos do node[0] e node[1] se tocam?
				faceProcFaceMask[dir][i][0], // filho do node[0]
				faceProcFaceMask[dir][i][1]  // filho do node[1]
			};

			// Para cada lado da face:
			// - Se o nó já é folha -> ele é o mais fino possível
			// - Se é interno → descemos para o filho correto que toca essa subface
			for (int j = 0; j < 2; j++)
			{
				if (node[j]->type != Node_Internal)
					faceNodes[j] = node[j];
				else
					faceNodes[j] = node[j]->children[c[j]];
			}

			// Recursão:
			// Continua descendo até que AMBOS os lados estejam no mesmo nível local
			ContourFaceProc(faceNodes, faceProcFaceMask[dir][i][2], indexBuffer);
		}

		// ------------------------------------------------------------------
		// ETAPA 2: PROCESSAR AS 4 ARESTAS INTERNAS DESSA FACE
		//
		// Após dividir a face em subfaces, surgem 4 arestas internas
		// Essas arestas são onde os cracks realmente aparecem
		const int orders[2][4] =
		{
			{ 0, 0, 1, 1 }, // padrão 0 → dois nós do lado 0, dois do lado 1
			{ 0, 1, 0, 1 }  // padrão 1 → intercalado
		};

		for (int i = 0; i < 4; i++)
		{
			DCNode* edgeNodes[4];

			// Para essa aresta interna:
			// quais filhos participam dela?
			const int c[4] =	//obs : só será necessario essa informação quando a aresta nao esta no no mais fino pois é necessario saber quais nós tocaram essa edge
			{
				faceProcEdgeMask[dir][i][1],
				faceProcEdgeMask[dir][i][2],
				faceProcEdgeMask[dir][i][3],
				faceProcEdgeMask[dir][i][4],
			};

			// Escolhe qual padrão de lados usar (node[0] ou node[1])
			const int* order = orders[faceProcEdgeMask[dir][i][0]];

			// Para cada um dos 4 nós que cercam a aresta:
			for (int j = 0; j < 4; j++)
			{
				// Se o nó já é folha (ou pseudo-leaf),
				// usamos ele diretamente
				if (node[order[j]]->type == Node_Leaf ||
					node[order[j]]->type == Node_Psuedo)
				{
					edgeNodes[j] = node[order[j]];
				}
				else
				{
					// Caso contrário, descemos para o filho correto
					edgeNodes[j] = node[order[j]]->children[c[j]];
				}
			}

			// Agora temos exatamente 4 células corretas ao redor da aresta
			// EdgeProc irá gerar os triângulos sem cracks
			ContourEdgeProc(edgeNodes, faceProcEdgeMask[dir][i][5], indexBuffer);
		}
	}
}


// ContourCellProc percorre a octree e inicia todo o processo de geração de malha
void ContourCellProc(DCNode* node, IndexBuffer& indexBuffer)
{
	if (!node) return;

	// Apenas nós internos participam:
	// folhas só armazenam vértices
	if (node->type != Node_Internal)
		return;

	// ------------------------------------------------------------------
	// ETAPA 1: RECURSÃO
	// Garante que os níveis mais finos sejam processados primeiro
	for (int i = 0; i < 8; i++)
		ContourCellProc(node->children[i], indexBuffer);

	// ------------------------------------------------------------------
	// ETAPA 2: PROCESSAR AS FACES INTERNAS ENTRE OS FILHOS
	//
	// Um cubo tem 12 pares de filhos que compartilham UMA FACE interna
	for (int i = 0; i < 12; i++)
	{
		DCNode* faceNodes[2];

		// Cada entrada da máscara indica:
		// - dois filhos que se tocam por uma face
		// - a direção dessa face (X, Y ou Z)
		const int c[2] = {
			cellProcFaceMask[i][0],
			cellProcFaceMask[i][1]
		};

		faceNodes[0] = node->children[c[0]];
		faceNodes[1] = node->children[c[1]];

		// Resolve LOD e subdivisões nessa face interna
		ContourFaceProc(faceNodes, cellProcFaceMask[i][2], indexBuffer);
	}

	// ------------------------------------------------------------------
	// ETAPA 3: PROCESSAR AS 6 ARESTAS INTERNAS DO CUBO
	//
	// Essas são arestas onde todos os 4 nós estão no mesmo nível
	for (int i = 0; i < 6; i++)
	{
		DCNode* edgeNodes[4];

		const int c[4] =
		{
			cellProcEdgeMask[i][0],
			cellProcEdgeMask[i][1],
			cellProcEdgeMask[i][2],
			cellProcEdgeMask[i][3],
		};

		for (int j = 0; j < 4; j++)
			edgeNodes[j] = node->children[c[j]];

		// Geração direta de triângulos
		ContourEdgeProc(edgeNodes, cellProcEdgeMask[i][4], indexBuffer);
	}
}


void GenerateVertexIndices(DCNode* node, VertexBuffer& vertexBuffer)
{
	if (!node) return; 
	 
	if (node->type != Node_Leaf) {								//(node->type == Node_Internal) 
		for (int i = 0; i < 8; i++) {   
			GenerateVertexIndices(node->children[i], vertexBuffer); 
		}
	} 

	if (node->type != Node_Internal)
	{
		OctreeDrawInfo* d = node->drawInfo;
		if (!d)
		{
			printf("Error! Could not add vertex!\n");
			exit(EXIT_FAILURE);
		}

		d->index = vertexBuffer.size();
		vertexBuffer.push_back(Vertex{ d->position, d->averageNormal });		//vertexBuffer.push_back(Vertex(d->position, d->averageNormal));
	} 
}

void GenerateMeshFromOctree(DCNode* node, VertexBuffer& vertexBuffer, IndexBuffer& indexBuffer)
{
	if (!node) return;

	vertexBuffer.clear();
	indexBuffer.clear();		//onde os triângulos finais serão escritos
	 
	GenerateVertexIndices(node, vertexBuffer); 
	ContourCellProc(node, indexBuffer);
}


/*
void GenerateVertexIndices(DCNode* node, VertexBuffer& vertexBuffer)
{
	if (!node) return;

	if (node->type != Node_Leaf) {
		for (int i = 0; i < 8; i++)
			if (node->children[i])
				GenerateVertexIndices(node->children[i], vertexBuffer);
	}

	if (node->type != Node_Internal && node->drawInfo)
	{
		OctreeDrawInfo* d = node->drawInfo;
		d->index = vertexBuffer.size();
		vertexBuffer.push_back(Vertex{ d->position, d->averageNormal });
	}
}
*/