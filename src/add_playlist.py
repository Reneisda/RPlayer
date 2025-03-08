import sys
import ytDlWrapper
from config import Config
from os import path as p
from pathlib import Path


if len(sys.argv) < 1:
    exit(1)

id_or_url = sys.argv[1]

if len(id_or_url) < 34:
    sys.exit(1)

CONFIG_PATH = p.join(Path.home(), ".config", "rplayer")
CONFIG_FILE_NAME = "rplayer.conf"
CONFIG_PATH_FULL = p.join(CONFIG_PATH, CONFIG_FILE_NAME)

con = Config(CONFIG_PATH_FULL)

id_or_url = id_or_url.replace("https://www.youtube.com/playlist?list=", "")

ytDlWrapper.add_new_playlist(id_or_url, con.conf["music_location"])
sys.exit(0)
