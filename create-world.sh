#! /bin/bash

sqlite3 iminiru.db < sql/planes.sql
sqlite3 iminiru.db < sql/planets.sql
sqlite3 iminiru.db < sql/rooms.sql

sqlite3 iminiru.db < iminiru/data.sql

