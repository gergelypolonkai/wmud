import json
from math import sqrt


TYPES = {}


class TileType(object):
    def __init__(self, data):
        self.map_char = data.pop('map_char')
        self.extra_data = data

    def __str__(self):
        return self.map_char


class Coordinate(object):
    def __init__(self, x, y):
        if x < 0 or y < 0:
            raise ValueError(f'Invalid coordinates [{x}; {y}]')

        self.__x = x
        self.__y = y

    @property
    def x(self):
        return self.__x

    @property
    def y(self):
        return self.__y

    def __str__(self):
        return f'[{self.x}; {self.y}]'


class Tile(object):
    def __init__(self, type):
        self.__type = type

    def __str__(self):
        return self.__type.char


class Plane(object):
    def __init__(self):
        self.__tiles = {}
        self.width = None
        self.height = 0

    def read(self, filename):
        with open(filename) as f:
            fields = f.read().split()

        self.width = None

        for row_num, row in enumerate(fields):
            # We simply skip completely empty rows
            if not row:
                continue

            if self.width is None:
                self.width = len(row)
            elif self.width != len(row):
                raise ValueError(f'Row {row_num} differs in length!')

            for col_num, field in enumerate(row):
                self.__tiles[Coordinate(col_num, row_num)] = get_type(field)

            self.height = row_num + 1

    def print(self, coords, radius):
        """Print a round part of the map, with `coords` in the midpoint, and the
        horizontal radius (ie. width in characters) `radius`

        This will result in a shape that resembles a circle, but, as text
        characters have about 1:2 ratio, is actually an ellipse.
        """

        if not isinstance(coords, Coordinate):
            coords = Coordinate(*coords)

        if radius > self.width // 2:
            raise ValueError('Radius is too big')

        if radius % 2 != 1:
            raise ValueError('Radius must be odd')

        width = (radius - 1) * 2 + 1

        mid_x, mid_y = width // 2, radius // 2

        for row in range(0, radius):
            y_offset = row - mid_y
            y = y_offset * 2
            map_y = coords.y + y_offset

            if map_y < 0:
                map_y += self.height
            elif map_y >= self.height:
                map_y -= self.height

            for column in range(0, width):
                x_offset = column - mid_x
                x = x_offset
                map_x = coords.x + x_offset

                if map_x < 0:
                    map_x += self.width
                elif map_x >= self.width:
                    map_x -= self.width

                if x < 0:
                    x -= 1
                elif x > 0:
                    x += 1

                distance = sqrt(y ** 2 + x ** 2)

                if row == mid_y and column == mid_x:
                    print('@', end='')
                elif distance > radius + 0.25:
                    print(' ', end='')
                else:
                    tile = self[(map_x, map_y)]
                    print(tile, end='')

            print('')

    def __getitem__(self, coord):
        if not isinstance(coord, Coordinate):
            coord = Coordinate(*coord)

        for key, tile in self.__tiles.items():
            if key.x == coord.x and key.y == coord.y:
                return tile

    def __str__(self):
        map_str = ''

        prev_y = -1

        for coord in sorted(self.__tiles.keys(), key=lambda coord: (coord.y, coord.x)):
            if coord.y != prev_y:
                prev_y = coord.y
                map_str += '\n'

            map_str += self.__tiles[coord].map_char

        return map_str

    def __repr__(self):
        return '<Map>'


def get_type(type_char):
    candidates = {identifier: data for identifier, data in TYPES.items() if data.map_char == type_char}

    if not candidates:
        raise KeyError(f'Map character "{type_char}" is unknown')

    if len(candidates) > 1:
        raise KeyError(f'Map character "{type_char}" has multiple definitions!')

    return candidates.popitem()[1]


def load_terrain(filename):
    with open(filename, 'r') as f:
        terrain_data = json.load(f)

    for identifier, data in terrain_data.items():
        # TODO: Validate terrain data
        tile_type = TileType(data)
        TYPES[identifier] = tile_type
