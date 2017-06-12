"""World definitions for wMUD
"""

from enum import IntEnum


class Directions(IntEnum):
    """All the available directions
    """

    NORTH = 0
    SOUTH = 1
    EAST = 2
    WEST = 3
    NORTH_EAST = 4
    SOUTH_WEST = 5
    NORTH_WEST = 6
    SOUTH_EAST = 7
    UP = 8
    DOWN = 9

    @property
    def opposite(self):
        """Get the opposite direction
        """

        if self % 2 == 0:
            return self.__class__(self + 1)

        return self.__class__(self - 1)
