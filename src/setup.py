from sys import platform
from pathlib import Path
from os import path as p
from os import makedirs
import config as cfg

CONFIG_PATH = p.join(Path.home(), ".config", "rplayer")
CONFIG_FILE_NAME = "rplayer.conf"

CONFIG_PATH_FULL = p.join(CONFIG_PATH, CONFIG_FILE_NAME)
DEFAULT_MUSIC_PATH = p.join(Path.home(), "Music", "rplayer")


def install():
    if p.exists(CONFIG_PATH_FULL):
       read_config()

    else:
        create_config()

def read_config():
    conf = cfg.Config(CONFIG_PATH_FULL)
    print(conf)


def create_config():
    makedirs(CONFIG_PATH, exist_ok=True)
    conf = cfg.Config(CONFIG_PATH_FULL)
    conf.write(
        {"volume" : 0.2,
         "music_location": DEFAULT_MUSIC_PATH
        })


if __name__ == "__main__":
    install()


    if platform == "linux" or platform == "linux2":
        pass
        #install_linux()

    elif platform == "win32":
        pass
        #install_windows()

