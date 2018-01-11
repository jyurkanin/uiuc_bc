ROBOT=jplayer
submit:
	mkdir -p ../../bc18-scaffold/${ROBOT}
	cp run.sh ../../bc18-scaffold/${ROBOT}
	cp *.cpp ../../bc18-scaffold/${ROBOT}
	cp *.hpp ../../bc18-scaffold/${ROBOT}
all:
	g++ *.cpp *.hpp `ncurses5-config --libs` -O3 -o bot
debug:
	g++ *.cpp *.hpp `ncurses5-config --libs` -D DEBUG -O3 -o bot
