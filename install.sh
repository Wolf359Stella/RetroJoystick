SCRIPT_DIR=$(which arduino)
CUR_DIR=$(pwd)

if [[ "$SCRIPT_DIR" == *snap* ]]; then
    TARGET_DIR="$HOME/snap/arduino/current/Arduino/libraries/retro_joystick"
    mkdir -p $TARGET_DIR 
    LINK_NAME="$TARGET_DIR/retro_joystick.cpp"
    ln -s "$CUR_DIR/retro_joystick.cpp" "$TARGET_DIR/retro_joystick.cpp"
    ln -s "$CUR_DIR/retro_joystick.h" "$TARGET_DIR/retro_joystick.h"
    ln -s "$CUR_DIR/examples" "$TARGET_DIR/examples"
    ln -s "$CUR_DIR/DynamicHID" "$TARGET_DIR/DynamicHID"
    echo "Symbolic links created"
else
    echo "Installation folder not found"
fi