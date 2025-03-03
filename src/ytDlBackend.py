
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
        self.from_file(SONG_PATH + "info.pl")

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

    def from_file(self, playlist_name):
        with open(PLAYLIST_PATH + playlist_name + ".pl") as f:
            ids = f.read().split("\n")

        assert ids
        for id_ in ids:
            self.songs.col.append(Song(id_, None, None))

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



if __name__ == "__main__":
    songs = SongCollection()
    songs.from_default()
    #print(songs)
    p = Playlist()
    p.from_file("Overnight")
    p.fetch_info(songs)
    print(p)