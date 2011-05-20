$! Xscreensaver - definition of various DCL symbols
$ set NOON
$ set def [.HACKS]
$ mydisk = f$trnlmn("SYS$DISK")
$ mydir  = mydisk+f$directory()
$ ant		:== $'mydir'ant
$ attraction	:== $'mydir'attraction
$ blaster	:== $'mydir'blaster
$ blitspin	:== $'mydir'blitspin
$ bouboule	:== $'mydir'bouboule
$ braid		:== $'mydir'braid
$ bsod		:== $'mydir'bsod
$ bubbles	:== $'mydir'bubbles
$ bumps		:== $'mydir'bumps
$ ccurve	:== $'mydir'ccurve
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
$ hyperball	:== $'mydir'hyperball
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
$ nerverot	:== $'mydir'nerverot
$ noseguy	:== $'mydir'noseguy
$ pedal		:== $'mydir'pedal
$ penetrate	:== $'mydir'penetrate
$ penrose	:== $'mydir'penrose
$ petri		:== $'mydir'petri
$ phosphor	:== $'mydir'phosphor
$ pyro		:== $'mydir'pyro
$ qix		:== $'mydir'qix
$ rd-bomb	:== $'mydir'rd-bomb
$ ripples	:== $'mydir'ripples
$ rocks		:== $'mydir'rocks
$ rorschach	:== $'mydir'rorschach
$ rotor		:== $'mydir'rotor
$ rotzoomer	:== $'mydir'rotzoomer
$ shadebobs	:== $'mydir'shadebobs
$ sierpinski	:== $'mydir'sierpinski
$ slidescreen	:== $'mydir'slidescreen
$ slip		:== $'mydir'slip
$ sonar		:== $'mydir'sonar
$ speedmine	:== $'mydir'speedmine
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
$ whirlwindwarp	:== $'mydir'whirlwindwarp
$ whirlygig	:== $'mydir'whirlygig
$ worm		:== $'mydir'worm
$ xflame	:== $'mydir'xflame
$ xjack		:== $'mydir'xjack
$ xlyap		:== $'mydir'xlyap
$ xmatrix	:== $'mydir'xmatrix
$ xrayswarm	:== $'mydir'xrayswarm
$ xspirograph	:== $'mydir'xspirograph
$ xsublim	:== $'mydir'xsublim
$ xteevee	:== $'mydir'xteevee
$ zoom		:== $'mydir'zoom
$ set def [-.DRIVER]
$ mydir  = mydisk+f$directory()
$ xscreensaver :== $'mydir'xscreensaver
$ xscreen*command :== $'mydir'xscreensaver-command
$ set def [-]
$ exit
