$! Xscreensaver - definition of various DCL symbols
$ set NOON
$ set def [.HACKS]
$ mydisk = f$trnlmn("SYS$DISK")
$ mydir  = mydisk+f$directory()
$ ant		:== $'mydir'ant
$ attraction	:== $'mydir'attraction
$ blitspin	:== $'mydir'blitspin
$ bouboule	:== $'mydir'bouboule
$ braid		:== $'mydir'braid
$ bsod		:== $'mydir'bsod
$ bubbles	:== $'mydir'bubbles
$ compass	:== $'mydir'compass
$ coral		:== $'mydir'coral
$ critical	:== $'mydir'critical
$ crystal	:== $'mydir'crystal
$ cynosure	:== $'mydir'cynosure
$ decayscreen	:== $'mydir'decayscreen
$ deco		:== $'mydir'deco
$ deluxe	:== $'mydir'deluxe
$ demon		:== $'mydir'demon
$ discrete	:== $'mydir'discrete
$ distort	:== $'mydir'distort
$ drift		:== $'mydir'drift
$ epicycle	:== $'mydir'epicycle
$ fadeplot	:== $'mydir'fadeplot
$ flag		:== $'mydir'flag
$ flame		:== $'mydir'flame
$ flow		:== $'mydir'flow
$ forest	:== $'mydir'forest
$ galaxy	:== $'mydir'galaxy
$ goop		:== $'mydir'goop
$ grav		:== $'mydir'grav
$ greynetic	:== $'mydir'greynetic
$ halo		:== $'mydir'halo
$ helix		:== $'mydir'helix
$ hopalong	:== $'mydir'hopalong
$ hypercube	:== $'mydir'hypercube
$ ifs		:== $'mydir'ifs
$ imsmap	:== $'mydir'imsmap
$ interference	:== $'mydir'interference
$ jigsaw	:== $'mydir'jigsaw
$ julia		:== $'mydir'julia
$ kaleidescope	:== $'mydir'kaleidescope
$ kumppa	:== $'mydir'kumppa
$ laser		:== $'mydir'laser
$ lightning	:== $'mydir'lightning
$ lisa		:== $'mydir'lisa
$ lissie	:== $'mydir'lissie
$ lmorph	:== $'mydir'lmorph
$ loop		:== $'mydir'loop
$ maze		:== $'mydir'maze
$ moire		:== $'mydir'moire
$ moire2	:== $'mydir'moire2
$ mountain	:== $'mydir'mountain
$ munch		:== $'mydir'munch
$ noseguy	:== $'mydir'noseguy
$ pedal		:== $'mydir'pedal
$ penetrate	:== $'mydir'penetrate
$ penrose	:== $'mydir'penrose
$ petri		:== $'mydir'petri
$ phosphor	:== $'mydir'phosphor
$ pyro		:== $'mydir'pyro
$ qix		:== $'mydir'qix
$ rd-bomb	:== $'mydir'rd-bomb
$ rocks		:== $'mydir'rocks
$ rorschach	:== $'mydir'rorschach
$ rotor		:== $'mydir'rotor
$ sierpinski	:== $'mydir'sierpinski
$ slidescreen	:== $'mydir'slidescreen
$ slip		:== $'mydir'slip
$ sonar		:== $'mydir'sonar
$ sphere	:== $'mydir'sphere
$ spiral	:== $'mydir'spiral
$ spotlight	:== $'mydir'spotlight
$ squiral	:== $'mydir'squiral
$ starfish	:== $'mydir'starfish
$ strange	:== $'mydir'strange
$ swirl		:== $'mydir'swirl
$ t3d		:== $'mydir't3d
$ triangle	:== $'mydir'triangle
$ truchet	:== $'mydir'truchet
$ vines		:== $'mydir'vines
$ wander	:== $'mydir'wander
$ worm		:== $'mydir'worm
$ xflame	:== $'mydir'xflame
$ xjack		:== $'mydir'xjack
$ xlyap		:== $'mydir'xlyap
$ xmatrix	:== $'mydir'xmatrix
$ xroger	:== $'mydir'xroger
$ set def [-.DRIVER]
$ mydir  = mydisk+f$directory()
$ xscreensaver :== $'mydir'xscreensaver
$ xscreen*command :== $'mydir'xscreensaver-command
$ set def [-]
$ exit
