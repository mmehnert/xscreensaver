$! Xscreensaver - definition of various DCL symbols
$ set NOON
$ set def [.HACKS]
$ mydisk = f$trnlmn("SYS$DISK")
$ mydir  = mydisk+f$directory()
$ abstractile	:== $'mydir'abstractile
$ anemone	:== $'mydir'anemone
$ anemotaxis	:== $'mydir'anemotaxis
$ apollonian	:== $'mydir'apollonian
$ apple2	:== $'mydir'apple2
$ attraction	:== $'mydir'attraction
$ barcode	:== $'mydir'barcode
$ blaster	:== $'mydir'blaster
$ blitspin	:== $'mydir'blitspin
$ bouboule	:== $'mydir'bouboule
$ boxfit	:== $'mydir'boxfit
$ braid		:== $'mydir'braid
$ bsod		:== $'mydir'bsod
$ bumps		:== $'mydir'bumps
$ ccurve	:== $'mydir'ccurve
$ celtic	:== $'mydir'celtic
$ cloudlife	:== $'mydir'cloudlife
$ compass	:== $'mydir'compass
$ coral		:== $'mydir'coral
$ crystal	:== $'mydir'crystal
$ cwaves	:== $'mydir'cwaves
$ cynosure	:== $'mydir'cynosure
$ decayscreen	:== $'mydir'decayscreen
$ deco		:== $'mydir'deco
$ deluxe	:== $'mydir'deluxe
$ demon		:== $'mydir'demon
$ discrete	:== $'mydir'discrete
$ distort	:== $'mydir'distort
$ drift		:== $'mydir'drift
$ epicycle	:== $'mydir'epicycle
$ eruption	:== $'mydir'eruption
$ euler2d	:== $'mydir'euler2d
$ fadeplot	:== $'mydir'fadeplot
$ fiberlamp	:== $'mydir'fiberlamp
$ fireworkx	:== $'mydir'fireworkx
$ flame		:== $'mydir'flame
$ flow		:== $'mydir'flow
$ fluidballs	:== $'mydir'fluidballs
$ fontglide	:== $'mydir'fontglide
$ fuzzyflakes	:== $'mydir'fuzzyflakes
$ galaxy	:== $'mydir'galaxy
$ goop		:== $'mydir'goop
$ grav		:== $'mydir'grav
$ greynetic	:== $'mydir'greynetic
$ halftone	:== $'mydir'halftone
$ halo		:== $'mydir'halo
$ helix		:== $'mydir'helix
$ hopalong	:== $'mydir'hopalong
$ ifs		:== $'mydir'ifs
$ imsmap	:== $'mydir'imsmap
$ interaggregate	:== $'mydir'interaggregate
$ interference	:== $'mydir'interference
$ intermomentary	:== $'mydir'intermomentary
$ julia		:== $'mydir'julia
$ kaleidescope	:== $'mydir'kaleidescope
$ kumppa	:== $'mydir'kumppa
$ lcdscrub	:== $'mydir'lcdscrub
$ loop		:== $'mydir'loop
$ m6502		:== $'mydir'm6502
$ maze		:== $'mydir'maze
$ memscroller	:== $'mydir'memscroller
$ metaballs	:== $'mydir'metaballs
$ moire		:== $'mydir'moire
$ moire2	:== $'mydir'moire2
$ mountain	:== $'mydir'mountain
$ munch		:== $'mydir'munch
$ nerverot	:== $'mydir'nerverot
$ noseguy	:== $'mydir'noseguy
$ pacman	:== $'mydir'pacman
$ pedal		:== $'mydir'pedal
$ penetrate	:== $'mydir'penetrate
$ penrose	:== $'mydir'penrose
$ petri		:== $'mydir'petri
$ phosphor	:== $'mydir'phosphor
$ piecewise	:== $'mydir'piecewise
$ polyominoes	:== $'mydir'polyominoes
$ pong		:== $'mydir'pong
$ popsquares	:== $'mydir'popsquares
$ pyro		:== $'mydir'pyro
$ qix		:== $'mydir'qix
$ rd-bomb	:== $'mydir'rd-bomb
$ ripples	:== $'mydir'ripples
$ rocks		:== $'mydir'rocks
$ rorschach	:== $'mydir'rorschach
$ rotzoomer	:== $'mydir'rotzoomer
$ shadebobs	:== $'mydir'shadebobs
$ sierpinski	:== $'mydir'sierpinski
$ slidescreen	:== $'mydir'slidescreen
$ slip		:== $'mydir'slip
$ speedmine	:== $'mydir'speedmine
$ spotlight	:== $'mydir'spotlight
$ squiral	:== $'mydir'squiral
$ starfish	:== $'mydir'starfish
$ strange	:== $'mydir'strange
$ substrate	:== $'mydir'substrate
$ swirl		:== $'mydir'swirl
$ thornbird	:== $'mydir'thornbird
$ triangle	:== $'mydir'triangle
$ truchet	:== $'mydir'truchet
$ twang		:== $'mydir'twang
$ vermiculate	:== $'mydir'vermiculate
$ wander	:== $'mydir'wander
$ webcollage-helper	:== $'mydir'webcollage-helper
$ whirlwindwarp	:== $'mydir'whirlwindwarp
$ wormhole	:== $'mydir'wormhole
$ xanalogtv	:== $'mydir'xanalogtv
$ xflame	:== $'mydir'xflame
$ xjack		:== $'mydir'xjack
$ xlyap		:== $'mydir'xlyap
$ xmatrix	:== $'mydir'xmatrix
$ xrayswarm	:== $'mydir'xrayswarm
$ xspirograph	:== $'mydir'xspirograph
$ zoom		:== $'mydir'zoom
$ set def [-.DRIVER]
$ mydir  = mydisk+f$directory()
$ xscreensaver :== $'mydir'xscreensaver
$ xscreen*command :== $'mydir'xscreensaver-command
$ set def [-]
$ exit
