#
# HyperCube OS
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Affero General Public License as
# published by the Free Software Foundation, either version 3 of the
# License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Affero General Public License for more details.
#
# You should have received a copy of the GNU Affero General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.
#
# 


mkdir bin/ 2> /dev/null
mkdir asan/ 2> /dev/null
gcc -m32 main.c state.c opcodes.c decompiler.c core.c dict.c -g -O3 -o bin/hexas_32
gcc -m32 main.c state.c opcodes.c decompiler.c core.c dict.c -g -O3 -DDECOMPILER -o bin/hexas_32_decompiler
gcc -m32 main.c state.c opcodes.c decompiler.c core.c dict.c -g -O3 -DDISASSEMBLER -o bin/hexas_32_disassembler
gcc -m32 main.c state.c opcodes.c decompiler.c core.c dict.c -g -O3 -DDECOMPILER -DLOOP -o bin/hexas_32_decompiler_loop
gcc -m32 main.c state.c opcodes.c decompiler.c core.c dict.c -g -O3 -DDISASSEMBLER -DLOOP -o bin/hexas_32_disassembler_loop
gcc -m32 main.c state.c opcodes.c decompiler.c core.c dict.c -g -O3 -DHYBRID -o bin/hexas_32_hybrid_loop
gcc -m32 main.c state.c opcodes.c decompiler.c core.c dict.c -g -O3 -DHYBRID -DLOOP -o bin/hexas_32_hybrid_loop

gcc -m32 main.c state.c opcodes.c decompiler.c core.c dict.c -g -O3 -fsanitize=address -DDECOMPILER -o bin/hexas_32_decompiler_asan
gcc -m32 main.c state.c opcodes.c decompiler.c core.c dict.c -g -O3 -fsanitize=address -DDECOMPILER -DLOOP -o bin/hexas_32_decompiler_loop_asan

gcc -m32 main.c state.c opcodes.c decompiler.c core.c dict.c -g -O3 -fsanitize=address -DHYBRID -o bin/hexas_32_hybrid_asan
gcc -m32 main.c state.c opcodes.c decompiler.c core.c dict.c -g -O3 -fsanitize=address -DHYBRID -DLOOP -o bin/hexas_32_hybrid_loop_asan

gcc -m32 generate_config.c -o bin/generate_config
