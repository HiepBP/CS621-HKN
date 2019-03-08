# User Guide

This document is a guide for users developing applications on NS-3 using Libdash library. This guide is not a comprehensive reference and is simply intended to help new users run NS-3 application for the first time.



## Pre-Requests

To run the application, you need to install these packages into your machine.

	sudo apt-get install haveged
	sudo apt-get install libboost-all-dev
	sudo apt-get install build-essential gccxml
	sudo apt-get install git-core build-essential cmake libxml2-dev libcurl4-openssl-dev
	sudo apt-get install cmake libxml2-dev libcurl4-openssl-dev
	sudo apt-get install libxml2-dev libxslt-dev python-dev lib32z1-dev

## Build

Build Libdash library:

	cd AMuSt-libdash/libdash
	mkdir build
	cd build
	cmake ../
	make

Build NS-3 Application

	cd ns-3-dev/
	./waf configure
	./waf build

## Running application

	./waf --run "p2p --link_capacity=10Mbps"

