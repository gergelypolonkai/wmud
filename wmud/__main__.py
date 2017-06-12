from wmud.world.map import Plane, load_terrain

load_terrain('data/terrain.json')
map = Plane()
map.read('data/plane.erodar.txt')
print(map)
print('')
map.print((0, 0), 11)
