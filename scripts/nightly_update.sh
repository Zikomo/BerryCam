sudo systemctl stop berrycam.service
cd "$BERRYCAM_ROOT" || exit
cp Release/settings.json temp
rm -rf Release
git fetch
get checkout master
git pull
mkdir Release
cd Release || exit
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j4
cp ../temp settings.json
sudo systemctl start berrycam.service

