from os import path as p, write
from os import makedirs


class Config:
    conf = {}
    path = None
    def __init__(self, path):
        self.path = path
        pairs = []
        if p.exists(path):
            with open(path, "r") as f:
                data = f.read().split("\n")
                for line in data:
                    if ":" in line:
                        pairs.append(tuple(line.split(":")))

            for pair in pairs:
                self.conf[pair[0].strip()] = pair[1].strip()

    def write(self, values : dict):
        if not self.path:
            print("No config path specified")
            return


        with open(self.path, "w") as f:
            for key, value in values.items():
                f.write(f"{key}: {value}\n")

    def __str__(self):
        return str(self.conf)