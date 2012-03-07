$!
$! VMS_CIRCLEMUD.COM
$! Written By:  Robert Alan Byer
$!              byer@mail.ourservers.net
$!
$! This command procedure is called from [-]VMS_AUTORUN.COM to run CIRCLE.EXE
$! as a standalone detached process.
$!
$! The CIRCLE.EXE accepts the following parameters.
$!
$!  	-m	Mini-Mud Mode.  Mini-mud will be one of your most powerful
$!		debugging tools; it causes Circle to boot with an abridged
$!		world, cutting the boot time down to several seconds.  It
$!		is useful for testing features with are not world-related.
$!		(i.e. new commands or spells)
$!
$!	-q	Quick boot.  Prevents checking of times out object files.
$!		Every time Circle boots, it checks every object file to
$!		see if it has timed out; if so, it is deleted.  This is
$!		done primarily to save disk space.  If time is more important
$!		to you than disk space, use the -q option.  "-q" is
$!		automatically activated when you use "-m"
$!
$!	-d	Data directory.  Useful as a debugging and development tool,
$!		if you want to keep one or more sets of game data in addition
$!		to the standard set, and choose which set is to be used at
$!		run-time.  Example, you can copy the entire world into a 
$!		seperate directory so you can test additions without 
$!		messing up the existing world files.  The default directory
$!		is "lib".  Any core dumps (may they never happen to you!) 
$!		will take place in the selected data directory.
$!
$!	port	Port selection.  Allows you to choose on which port the 
$!		game is to listen for connections.  The default is 4000,
$!		which is historically the port used by most DikuMUDs.
$!
$!
$! Define The Symbol For CIRCLE.EXE
$!
$ CIRCLE :== $CIRCLEMUD_ROOT:[BIN]CIRCLE.EXE
$!
$! Run CircleMUD.
$!
$ CIRCLE
$!
$! Time To Exit This Procedure.
$!
$ EXIT
