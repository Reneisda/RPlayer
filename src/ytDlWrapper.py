from os.path import isfile, join

from config import Config
from os import path as p
from pathlib import Path
import yt_dlp
import os
from PIL import Image


PLAYLIST_PATH = "../playlists/"
SONG_PATH = "../songs/"


class Song:
    def __init__(self, song_id, title, artist):
        self.title = title
        self.artist = artist
        self.id = song_id

    def __str__(self):
        return f"Title: {self.title}\n\tArtist: {self.artist}\n\tID: {self.id}\n"


class SongCollection:
    col = []
    def __init__(self):
        pass

    def from_default(self):
        self.from_file(SONG_PATH + "info.playlist")

    def from_file(self, filename):
        s = []
        with open(filename, "r") as f:
            lines = f.readlines()
            val = [None, None, None]
            vals_read = 0
            for line in lines:
                val[vals_read] = line.strip()
                vals_read += 1
                if vals_read == 3:
                    s.append(Song(val[0], val[1], val[2]))
                    vals_read = 0

        self.col = s

    def __len__(self):
        return len(self.col)


    def __str__(self):
        if len(self.col) == 0:
            return ""

        return "".join(f"{str(x)}: {str(s)}" for x, s in enumerate(self.col))


class Playlist:
    songs = SongCollection()
    name = None
    def from_file(self, dir):
        with open(dir) as f:
            ids = f.read().replace("\r", "").split("\n")

        assert ids
        self.name = ids[0]
        ids = ids[1:]
        for id_ in ids:
            self.songs.col.append(Song(song_id=id_, title=None, artist=None))

    def fetch_info(self, song_collection : SongCollection):
        all_ids = [s.id for s in song_collection.col]
        for i, song in enumerate(self.songs.col):
            if song.id not in all_ids:
                print(f"{song.id} not in Collection")
                continue

            self.songs.col.pop(i)
            self.songs.col.insert(i, song_collection.col[all_ids.index(song.id)])


    def __str__(self):
        return str(self.songs)


def download_song(song_id : str, base_dir : str):
    url = "https://www.youtube.com/watch?v=" + song_id
    song_folder = p.join(base_dir, "songs")
    thumbnails_folder = p.join(base_dir, "thumbnails")
    ydl_opts = {
        "format": "bestaudio/best",  # Get best audio format
        "outtmpl": f"{song_folder}/%(id)s.%(ext)s",  # Save file as video_id
        "postprocessors": [
            {
                "key": "FFmpegExtractAudio",
                "preferredcodec": "mp3",  # Convert to WAV
                "preferredquality": "192",  # Audio quality
            }
        ],
        "writethumbnail": True,  # Download thumbnails
        "outtmpl_thumbnail": f"{thumbnails_folder}/%(id)s.%(ext)s",
        "quiet": False,  # Show progress
        "ignoreerrors": True,  # Skip unavailable videos
        "extract_flat": False,
    }

    with yt_dlp.YoutubeDL(ydl_opts) as ydl:
        ydl.download([url])

    check_thumbnails(base_dir=base_dir)


def add_new_playlist(pl_id : str, base_dir : str):
    url = "https://www.youtube.com/playlist?list=" + pl_id
    song_folder = p.join(base_dir, "songs")
    thumbnails_folder = p.join(base_dir, "thumbnails")
    playlist_folder = p.join(base_dir, "playlists")
    ydl_opts = {
        "format": "bestaudio/best",  # Get best audio format
        "outtmpl": f"{song_folder}/%(id)s.%(ext)s",  # Save file as video_id
        "postprocessors": [
            {
                "key": "FFmpegExtractAudio",
                "preferredcodec": "mp3",  # Convert to WAV
                "preferredquality": "192",  # Audio quality
            }
        ],
        "writethumbnail": True,  # Download thumbnails
        "outtmpl_thumbnail": f"{thumbnails_folder}/%(id)s.%(ext)s",
        "quiet": False,  # Show progress
        "ignoreerrors": True,  # Skip unavailable videos
        "extract_flat": False,
    }

    with yt_dlp.YoutubeDL({"quiet": True, "extract_flat": True}) as ydl:
        info = ydl.extract_info(url, download=False)

    if not info or "entries" not in info:
        print("Error: Unable to fetch playlist details.")
        return

    video_ids = [video["id"] for video in info["entries"] if "id" in video]

    playlist_ids_file = os.path.join(playlist_folder, pl_id + ".playlist")
    with open(playlist_ids_file, "w") as f:
        f.write(info["title"] + "\n")
        f.write("\n".join(video_ids))

    print(f"Playlist video IDs saved to: {playlist_ids_file}")

    with yt_dlp.YoutubeDL(ydl_opts) as ydl:
        ydl.download([url])

    print("Download completed!")
    update_meta_data(base_dir)
    check_thumbnails(base_dir=base_dir)


def check_thumbnails(base_dir : str):
    thumbnails_folder = p.join(base_dir, "thumbnails")
    songs_folder = p.join(base_dir, "songs")
    os.makedirs(thumbnails_folder, exist_ok=True)

    # Loop through all files in the folder
    for file in os.listdir(songs_folder):
        if file.endswith(".webp"):
            webp_path = os.path.join(songs_folder, file)
            jpg_filename = os.path.splitext(file)[0] + ".jpg"
            jpg_path = os.path.join(thumbnails_folder, jpg_filename)

            try:
                # Open and convert the image
                img = Image.open(webp_path).convert("RGB")
                img.save(jpg_path, "JPEG", quality=95)

                # Remove the original .webp file
                os.remove(webp_path)
                print(f"Converted and moved: {file} → {jpg_path}")

            except Exception as e:
                print(f"Failed to convert {file}: {e}")

    print("Moving missing files")
    for file in os.listdir(songs_folder):
        if file.endswith(".jpg"):
            print(f"Moving {file}...")
            os.rename(os.path.join(songs_folder, file), os.path.join(thumbnails_folder, file))

    print("Fixing Done!")

# checks and fixes playlist. Removes songs that failed to download
def check_playlist(pl_id: str, base_dir : str):
    print("Checking playlists...")
    songs_in_list = SongCollection()
    songs_in_list.from_file(os.path.join(base_dir, "songs", "songs.info"))

    playlist_dir = p.join(base_dir, "playlists", pl_id + ".playlist")

    pl = Playlist()
    pl.from_file(playlist_dir)
    pl.fetch_info(songs_in_list)
    actual_songs = os.listdir(os.path.join(base_dir, "songs"))
    actual_songs.remove("songs.info")

    print(f"Checking {pl.name}")
    for i in range(len(actual_songs)):
        actual_songs[i] = actual_songs[i][:-4]

    print(actual_songs)

    for song in pl.songs.col:
        if song.id not in songs_in_list.col:
            print(f"Removing: {song.id} because meta-data incomplete")
            songs_in_list.col.remove(song)

    for song in pl.songs.col:
        if song.id not in actual_songs:
            print(f"Removing: {song.id} because download failed.")
            songs_in_list.col.remove(song)
    

def update_playlist(pl_id: str, base_dir : str):
    with open(p.join(base_dir, "playlists", pl_id + ".playlist"), "r") as f:
        current_ids = f.read().replace("\r", "").split("\n")

    title = current_ids.pop(0)
    url = "https://www.youtube.com/playlist?list=" + pl_id
    with yt_dlp.YoutubeDL({"quiet": True}) as ydl:
        info = ydl.extract_info(url, download=False)
        updated_ids = [video["id"] for video in info["entries"] if video]

    counter = 0
    for i in updated_ids:
        if i not in current_ids:
            current_ids.append(i)
            counter += 1
            download_song(i, base_dir)

    print(f"Found {counter} new songs in playlist {pl_id}")

    with open(p.join(base_dir, "playlists", pl_id + ".playlist"), "w") as f:
        f.write(title + "\n")
        f.write("\n".join(current_ids))

    print(f"Updating done!")
    update_meta_data(base_dir)


def update_meta_data(base_dir : str):
    songs = []
    songs_folder = p.join(base_dir, "songs")
    songs_info_file = os.path.join(songs_folder, "songs.info")
    if os.path.exists(songs_info_file):
        with open(songs_info_file, "r") as song_info:
            songs = song_info.read().split("\n")

    i = 0
    while i < len(songs) - 1:
        i += 1
        songs.pop(i)
        songs.pop(i)

    ydl_opts = {
        "extract_flat": True,
    }

    files = [f for f in os.listdir(songs_folder) if isfile(join(songs_folder, f))]
    info_file = open(songs_info_file, "a")
    counter = 0
    for file in files:
        file = file[:-4]
        if file in songs:
            continue

        if len(file) != 11:
            continue

        counter += 1
        with yt_dlp.YoutubeDL(ydl_opts) as ydl:
            info = ydl.extract_info(file, download=False)
            assert (info is not None)
            info_file.write(f'{info["id"]}\n')
            info_file.write(f'{info["title"]}\n')
            info_file.write(f'{info["uploader"]}\n')

    info_file.close()
    print(f"Updated {counter} songs")


def remove_playlist(pl_id: str, base_dir : str):
    pass


if __name__ == "__main__":
    BASE_DIR = p.join(Path.home(), "Music", "rplayer")

    CONFIG_PATH = p.join(Path.home(), ".config", "rplayer")
    CONFIG_FILE_NAME = "rplayer.conf"
    CONFIG_PATH_FULL = p.join(CONFIG_PATH, CONFIG_FILE_NAME)
    cfg = Config(CONFIG_PATH_FULL)
    check_playlist("PLF452AA85EDA3A598", cfg.conf["music_location"])
    #add_new_playlist("PLATB9XS5FOa4pwSRJ_zzrNU4aFxhKqzFp", BASE_DIR)
    #update_playlist("PLATB9XS5FOa4pwSRJ_zzrNU4aFxhKqzFp", BASE_DIR)
