#!/bin/bash

if [[ $EUID -ne 0 ]]; then
    echo "You must be a root user" 2>&1 
    exit 1
fi
mkdir /usr/share/translator
cp translator /usr/share/translator
cp icon.jpg /usr/share/translator
cp makefile /usr/share/translator
ln -s /usr/share/translator/translator /usr/bin/translator

