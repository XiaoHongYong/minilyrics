mkdir /tmp/ml_release
rm -r /tmp/ml_release/*
rm ~/zikiplayer/Install_ML_mac/Release/MiniLyrics.dmg
cp -R ~/zikiplayer/Install_ML_mac/Release/MiniLyrics.pkg /tmp/ml_release/
hdiutil create /tmp/tmp.dmg -ov -volname "MiniLyrics" -fs HFS+ -srcfolder /tmp/ml_release
hdiutil convert /tmp/tmp.dmg -format UDZO -o ~/zikiplayer/Install_ML_mac/Release/MiniLyrics.dmg
rm /tmp/tmp.dmg
