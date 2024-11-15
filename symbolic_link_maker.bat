mkdir components

IF EXIST components\ai-thinker-esp32-a1s(
    del components\ai-thinker-esp32-a1s
)
IF EXIST %dir_path% (
    rmdir /S /Q components\ai-thinker-esp32-a1s 
)

mklink /D components\ai-thinker-esp32-a1s %HOMEPATH%\esp\esp-adf\components\audio_board\esp-adf-component-ai-thinker-esp32-a1s\components\ai-thinker-esp32-a1s