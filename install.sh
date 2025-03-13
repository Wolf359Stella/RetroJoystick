SCRIPT_DIR=$(which arduino)
CUR_DIR=$(pwd)

if [[ "$SCRIPT_DIR" == *snap* ]]; then
    TARGET_DIR="$HOME/snap/arduino/current/Arduino/libraries/RetroJoystick"
    mkdir -p $TARGET_DIR 
    ln -s "$CUR_DIR/src" "$TARGET_DIR/src"
    ln -s "$CUR_DIR/examples" "$TARGET_DIR/examples"
    ln -s "$CUR_DIR/library.properties" "$TARGET_DIR/library.properties"
    echo "Symbolic links created"
else
    echo "Installation folder not found"
fi