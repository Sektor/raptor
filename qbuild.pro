# This is an application
TEMPLATE=app

# The binary name
TARGET=raptor

# This app uses Qtopia
CONFIG+=qtopia
DEFINES+=QTOPIA

# I18n info
STRING_LANGUAGE=en_US
LANGUAGES=en_US

HEADERS=\
    src/raptor.h

SOURCES=\
    src/main.cpp \
    src/raptor.cpp

# Package info
pkg [
    name=raptor
    desc="Lightweight GUI for APT"
    license=GPLv3
    version=1.0
    maintainer="Anton Olkhovik <ant007h@gmail.com>"
]

target [
    hint=sxe
    domain=untrusted
]

desktop [
    hint=desktop
    files=raptor.desktop
    path=/apps/Settings
]

pics [
    hint=pics
    files=pics/*
    path=/pics/raptor
]

help [
    hint=help
    source=help
    files=*.html
]
