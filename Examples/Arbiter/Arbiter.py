""" Tulip to ULM chart: Arbiter 
CREDIT

    Scott1. Piterman, Nir et al.: Synthesis of Reactive(1) Designs. 2006
"""

from __future__ import print_function

from tulip import spec, hybrid
import synth2 as synth
import Interface.Statechart as dumpsmach
from Reduce import *
import Interface.DSL as DSL
from itertools import combinations
# n input lines & n output lines
n = 5

# define boolean variables for the requests & the grants
r = []
g = []
for i in range(n):
    r += ['r' + str(i + 1)] # requests
    g += ['g' + str(i + 1)] # grants

env_vars, sys_vars, env_init, sys_init = set(), set(), set(), set()
env_safe, sys_safe, env_prog, sys_prog = set(), set(), set(), set()

# pure GR1 assumptions on the environment
for i in range(n):
    env_vars |= {r[i]}
    env_init |= {'!' + r[i]}  # initially all requests are at zeo
    env_safe |= {' ( ( !' + r[i] + ' && ' + g[i] + ') || ( !' + g[i] + ' && ' + r[i] + ') )'
                 + ' -> ' + ' ( ' + r[i] + ' <-> ' + ' X ' + r[i] + ' ) '}
    env_prog |= {'! ( ' + r[i] + ' && ' + g[i] + ')'}

# pure GR1 assumptions on the system
sys_safe |= {' && '.join(['!( ' + g[i] + ' && ' + g[j] + ' ) ' for (i,j) in combinations(range(n),2)]) }

for i in range(n):
    sys_vars |= {g[i]}
    sys_init |= {'!' + g[i]} # initially no grants are given
    sys_safe |= {'(' + r[i] + ' <-> ' + g[i] + ') -> ' + '(' + g[i] + ' <-> (X ' + g[i] + '))'}
    sys_prog |= {'(' + r[i] + ' <-> ' + g[i] + ')'}
psi = spec.GRSpec(env_vars, sys_vars, env_init, sys_init,
                  env_safe, sys_safe, env_prog,  sys_prog)

print(psi.pretty())
# # response LTL pattern for environment
# for i in range(n):
#     psi |= DSL.response(trig='(' + r[i] + ' && ' + g[i] + ')', react='( !' + r[i] + ')',
#                         owner='env', aux='env' + str(i+1))
#
# # response LTL pattern for sys
# for i in range(n):
#     psi |= DSL.response(trig='(' + r[i] + ' && !' + g[i] + ')', react='( ' + g[i] + ')',
#                         owner='sys', aux='sysa' + str(i+1))
#     psi |= DSL.response(trig='(!' + r[i] + ' && ' + g[i] + ')', react='(! ' + g[i] + ')',
#                         owner='sys', aux='sysb' + str(i+1))
#


psi.qinit = '\A \E'
psi.moore = False
psi.plus_one = False

ctrl = synth.synthesize(psi, ignore_sys_init=False, solver='gr1c')
# found a controller
print('controller found')

ctrl_red = reduce_mealy(ctrl, relabel=True, outputs=set(g),
                       prune_set=None, combine_trans=False, full=True)

save_png(ctrl_red,'Arbit_' + str(n) + '_lines' )


print("---------------------\n Arbiter  \n------------")

string_long = dumpsmach.mealy_to_xmi_uml(ctrl_red, outputs=set(g), name="Arbiter", relabel=False)

with open("Arbit.xml", "w") as f:
    f.write(string_long)
