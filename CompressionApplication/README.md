sudo apt-get install zlib1g-dev  

Pre-Requesits (Todo: cleanup)  
sudo apt-get install build-essential gccxml  
sudo apt-get install git-core build-essential cmake libxml2-dev libcurl4-openssl-dev  
sudo apt-get install cmake libxml2-dev libcurl4-openssl-dev  
sudo apt-get install libxml2-dev libxslt-dev python-dev lib32z1-dev  

Build  
cd AMuSt-libdash/libdash  
mkdir build  
cd build  
cmake ../  
make  
