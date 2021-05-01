# replace brave with your browser of choice like firefox or chrome
browser = brave
# replace cd with pwd on linux?
current_dir = $(shell cd)

all: vlearn

clean:
	vapm task clean

vlearn:
	vapm task debug debug_test gen_pdb

vlearn_release: # no need to gen here, release does not have debug symbols.
	vapm task release release_test



doc: FORCE
	vapm task doc
	cmd /C start $(browser) $(current_dir)\doc\index.html
FORCE: