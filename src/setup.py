#!/usr/bin/env pyhon
import sys
from sys import platform
from pathlib import Path
from os import path as p
from os import makedirs
import config as cfg
from shutil import which

CONFIG_PATH = p.join(Path.home(), ".config", "rplayer")
CONFIG_FILE_NAME = "rplayer.conf"

CONFIG_PATH_FULL = p.join(CONFIG_PATH, CONFIG_FILE_NAME)
DEFAULT_MUSIC_PATH = p.join(Path.home(), "Music", "rplayer")

DEPS = ["yt-dlp", "ffmpeg"]


def check_deps():
    found_all_deps = True
    for dep in DEPS:
        print(f"Checking for {dep} ...", end="\t")
        if which(dep) is not None:
            print("found!")
        else:
            found_all_deps = False
            print("not found!")

    return found_all_deps


def create_folder_structure(path):
    makedirs(p.join(path), exist_ok=True)
    makedirs(p.join(path, "songs"), exist_ok=True)
    makedirs(p.join(path, "playlists"), exist_ok=True)
    makedirs(p.join(path, "thumbnails"), exist_ok=True)


def install():
    print("Starting Installation...")
    if not check_deps():
        print("Error installing, missing dependencies found!")
        sys.exit(1)

    print("Found all dependencies!")

    if p.exists(CONFIG_PATH_FULL):
       config = read_config()
       if config["music_location"] is None:
            print("Found broken config\nWriting default config...")
            create_config()
    else:
        create_config()

    config = read_config()
    if config["music_location"] is None:
        print("Failed to find or write config file!")
        sys.exit(1)

    print("Creating music folder")
    create_folder_structure(config["music_location"])
    print("Installation complete!")
    sys.exit(0)


def read_config():
    conf = cfg.Config(CONFIG_PATH_FULL)
    return conf.conf


def create_config():
    makedirs(CONFIG_PATH, exist_ok=True)
    conf = cfg.Config(CONFIG_PATH_FULL)
    conf.write(
        {"volume" : 0.2,
         "music_location": DEFAULT_MUSIC_PATH + "/"
        })


if __name__ == "__main__":
    install()
