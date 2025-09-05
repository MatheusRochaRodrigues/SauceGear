#pragma once

// ==== Material Design Icons (Unicode) ====
// Fonte: Google Material Icons (TTF)
// Para usar: carregue a fonte no ImGui com o range unicode adequado (0xE000–0xF8FF).
// Exemplo de uso: ImGui::Button(ICON_MD_PLAY " Play");
// Obs: precisa ter a fonte "MaterialIcons-Regular.ttf" carregada.

#define ICON_MD_HOME            u8"\uE87C"
#define ICON_MD_SETTINGS        u8"\uE8B8"
#define ICON_MD_ADD             u8"\uE145"
#define ICON_MD_SAVE            u8"\uE161"
#define ICON_MD_OPEN            u8"\uE2C7"
#define ICON_MD_CLOSE           u8"\uE5C9"
#define ICON_MD_DELETE          u8"\uE872"
#define ICON_MD_EDIT            u8"\uE3C9"
#define ICON_MD_REFRESH         u8"\uE5D5"
#define ICON_MD_SEARCH          u8"\uE8B6"
#define ICON_MD_FILE            u8"\uE24D"
#define ICON_MD_FOLDER          u8"\uE2C7"
#define ICON_MD_FOLDER_OPEN     u8"\uE2C8"
#define ICON_MD_PLAY            u8"\uE037"
#define ICON_MD_PAUSE           u8"\uE034"
#define ICON_MD_STOP            u8"\uE047"
#define ICON_MD_ARROW_BACK      u8"\uE5C4"
#define ICON_MD_ARROW_FORWARD   u8"\uE5C8"
#define ICON_MD_UPLOAD          u8"\uE2C6"
#define ICON_MD_DOWNLOAD        u8"\uE2C4"
#define ICON_MD_WARNING         u8"\uE002"
#define ICON_MD_INFO            u8"\uE88E"
#define ICON_MD_CHECK           u8"\uE5CA"
#define ICON_MD_CANCEL          u8"\uE5CD"
#define ICON_MD_FULLSCREEN      u8"\uE5D0"
#define ICON_MD_FULLSCREEN_EXIT u8"\uE5D1"
#define ICON_MD_VISIBILITY      u8"\uE8F4"
#define ICON_MD_VISIBILITY_OFF  u8"\uE8F5"
#define ICON_MD_LIGHT_MODE      u8"\uE518"
#define ICON_MD_DARK_MODE       u8"\uE51C"
#define ICON_MD_BUILD           u8"\uE869"
#define ICON_MD_CODE            u8"\uE86F"
#define ICON_MD_VIEW_LIST       u8"\uE8EF"
#define ICON_MD_VIEW_MODULE     u8"\uE8F0"
#define ICON_MD_UNDO            u8"\uE166"
#define ICON_MD_REDO            u8"\uE15A"
#define ICON_MD_ZOOM_IN         u8"\uE8FF"
#define ICON_MD_ZOOM_OUT        u8"\uE900"








#pragma once

// Material Icons - Regular (https://fonts.google.com/icons)
// Necessário: MaterialIcons-Regular.ttf na pasta fonts/
// Uso: io.Fonts->AddFontFromFileTTF("fonts/MaterialIcons-Regular.ttf", 16.0f, nullptr, icons_ranges);

#define ICON_MIN_MI 0xE000
#define ICON_MAX_MI 0xF8FF

// Exemplos principais (alguns ícones comuns)
#define ICON_MI_FILE_IMPORT      u8"\uE2C6" // "file_upload"
#define ICON_MI_SCISSORS         u8"\uE3C6" // "content_cut"
#define ICON_MI_COPY             u8"\uE14D" // "content_copy"
#define ICON_MI_PASTE            u8"\uE16D" // "content_paste"

#define ICON_MI_PLUS             u8"\uE145" // "add"
#define ICON_MI_FOLDER           u8"\uE2C7" // "folder"
#define ICON_MI_FILE             u8"\uE24D" // "insert_drive_file"
#define ICON_MI_BOX              u8"\uE2C4" // "inventory_2"
#define ICON_MI_FILL_DRIP        u8"\uE3B0" // "format_paint"
#define ICON_MI_IMAGE            u8"\uE3F4" // "image"
#define ICON_MI_MUSIC            u8"\uE405" // "music_note"
#define ICON_MI_FILE_CODE        u8"\uE86F" // "code"
#define ICON_MI_FILM             u8"\uE04A" // "movie"

#define ICON_MI_SYNC             u8"\uE863" // "sync"
#define ICON_MI_FOLDER_OPEN      u8"\uE2C8" // "folder_open"
