#pragma once
#include <filesystem>
#include <string>

//Asset é tudo aquilo que compoem os dados para ser capaz de ser processado pela Engine
// Instance que é direcionado a função de rodar aquele Asset na Cena como por exemplo levar a GPU

/*
* Asset  = dado imutável vindo de arquivo
* Instance = estado vivo na cena 
* Renderer = só consome instance
*/

struct IAsset {
    std::string path;
    std::filesystem::file_time_type lastWrite;

    virtual ~IAsset() = default;
    virtual void Reload() = 0;
};
