import yt_dlp
from os import listdir
from os.path import isfile, join
import os
import PIL as Image


thumbnails_folder = "thumbnails"
import os
from PIL import Image


def convert_thumbnails(folder):
    thumbnails_folder = os.path.abspath(os.path.join(folder, "..", "thumbnails"))
    os.makedirs(thumbnails_folder, exist_ok=True)

    # Loop through all files in the folder
    for file in os.listdir(folder):
        if file.endswith(".webp"):
            webp_path = os.path.join(folder, file)
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


def download_playlist_as_wav(playlist_url, cookies_file=None, output_folder="songs"):
    ydl_opts = {
        "format": "bestaudio/best",  # Get best audio format
        "outtmpl": f"{output_folder}/%(id)s.%(ext)s",  # Save file as video_id
        "postprocessors": [
            {
                "key": "FFmpegExtractAudio",
                "preferredcodec": "wav",  # Convert to WAV
                "preferredquality": "192",  # Audio quality
            }
        ],
        "writethumbnail": True,  # Download thumbnails
        "outtmpl_thumbnail": f"../{thumbnails_folder}/%(id)s.%(ext)s",
        "quiet": False,  # Show progress
        "ignoreerrors": True,  # Skip unavailable videos
        "extract_flat": False,  # Ensure full playlist extraction
    }

    if cookies_file:
        ydl_opts["cookiefile"] = cookies_file

    with yt_dlp.YoutubeDL({"quiet": True, "extract_flat": True}) as ydl:
        info = ydl.extract_info(playlist_url, download=False)

    if not info or "entries" not in info:
        print("Error: Unable to fetch playlist details.")
        return

    video_ids = [video["id"] for video in info["entries"] if "id" in video]

    playlist_ids_file = os.path.join("playlists", info["title"] + ".pl")
    with open(playlist_ids_file, "w") as f:
        f.write("\n".join(video_ids))

    print(f"Playlist video IDs saved to: {playlist_ids_file}")

    with yt_dlp.YoutubeDL(ydl_opts) as ydl:
        ydl.download([playlist_url])

    print("Download completed!")   

   
def write_playlist_info(playlist_url):
    ydl_opts = {
        "quiet": True,
        "extract_flat": True, 
    }

    with yt_dlp.YoutubeDL(ydl_opts) as ydl:
        info = ydl.extract_info(playlist_url, download=False)

        if not info or "entries" not in info:
            print("Failed to retrieve playlist info.")
            return

        videos = info["entries"]
        pl_title = info["title"]
        data = pl_title + "\n"
        with open(f"playlists/{pl_title}.pl", "w") as pl_file:
            for video in videos:
                data += video.get("id", "Unknown") + "\n"
                data += video.get("title", "Unknown") + "\n"
                data += video.get("uploader", "Unknown") + "\n"

            pl_file.write(data)


def update_meta_data():
    songs = []
    if os.path.exists("songs/info.pl"):
        with open("songs/info.pl", "r") as song_info:
            songs = song_info.read().split("\n")

    i = 0
    while i < len(songs) - 1:
        i += 1 
        songs.pop(i)
        songs.pop(i)
        
    ydl_opts = {
            "extract_flat": True, 
            }
    
    files = [f for f in listdir("songs") if isfile(join("songs", f))]
    info_file = open("songs/info.pl", "a")
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
            assert(info is not None)
            info_file.write(f'{info["id"]}\n')
            info_file.write(f'{info["title"]}\n')
            info_file.write(f'{info["uploader"]}\n')

    info_file.close() 
    print(f"Updated {counter} songs")


playlist_url = "https://www.youtube.com/playlist?list=OLAK5uy_mTVjCbTPFRuHHlP1IS2fo_1sQNBWK1ciY"
download_playlist_as_wav(playlist_url, output_folder="songs/")
convert_thumbnails("songs")
update_meta_data()

