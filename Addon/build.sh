#!/bin/sh
ADDON=HB-UNI-Sen-WEA
rm $ADDON-addon.tgz
find . -name ".DS_Store" -exec rm -rf {} \;
cd $ADDON-addon-src
source ./addon/params 
echo ${ADDON_VERSION} > ./addon/VERSION

rm ./addon/update-check.cgi
if [ -n "${CHECK_URL}" ]; then
echo "#!/bin/tclsh" > ./addon/update-check.cgi
echo "set checkURL    \"${CHECK_URL}\"" >> ./addon/update-check.cgi
echo "set downloadURL \"${DOWNLOAD_URL}\"" >> ./addon/update-check.cgi
cat ../update-check.cgi.skel >> ./addon/update-check.cgi
fi 
        
chmod +x update_script
chmod +x addon/install*
chmod +x addon/update-check.cgi
chmod +x rc.d/*
tar -zcvf ../$ADDON-addon.tgz *
cd ..
