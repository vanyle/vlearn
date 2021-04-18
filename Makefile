# replace brave with your browser of choice like firefox or chrome
browser = brave
# replace cd with pwd on linux?
current_dir = $(shell cd)

all: vtoolbox

clean:
	vapm task clean

vtoolbox:
	vapm task debug release_test genpdb



doc: FORCE
	vapm task doc
	cmd /C start $(browser) $(current_dir)\doc\index.html
FORCE: