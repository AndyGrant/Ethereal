/*
  Ethereal is a UCI chess playing engine authored by Andrew Grant.
  <https://github.com/AndyGrant/Ethereal>     <andrew@grantnet.us>

  Ethereal is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Ethereal is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

// Disable this picky gcc-8 compiler warning
#if defined(__GNUC__) && (__GNUC__ >= 8)
#pragma GCC diagnostic ignored "-Wcast-function-type"
#endif

#include "windows.h"

#ifndef _WIN32

void bindThisThread(int index) { (void)index; };

#else

static int bestGroup(int index) {

    // bestGroup() retrieves logical processor information using Windows specific
    // API and returns the best group id for the thread with a given index. Original
    // code from Texel by Peter Österlund. Current code from Stockfish authors.

    int groupSize = 0, groups[2048];
    int nodes = 0, cores = 0, threads = 0;
    DWORD returnLength = 0, byteOffset = 0;

    // Early exit if the needed API is not available
    HMODULE k32 = GetModuleHandle("Kernel32.dll");
    fun1_t fun1 = (fun1_t)GetProcAddress(k32, "GetLogicalProcessorInformationEx");
    if (!fun1) return -1;

    // First call to get returnLength. We expect
    // it to fail due to the NULL buffer we pass
    if (fun1(RelationAll, NULL, &returnLength))
        return -1;

    // Once we know returnLength, allocate the buffer
    SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX *buffer, *ptr;
    ptr = buffer = malloc(returnLength);

    // Second call, now we expect to succeed
    if (!fun1(RelationAll, buffer, &returnLength))
        return free(buffer), -1;

    // Count up all nodes, cores, and threads (assume 2 threads max)
    while (byteOffset < returnLength) {

        if (ptr->Relationship == RelationNumaNode)
            nodes++;

        else if (ptr->Relationship == RelationProcessorCore)
            cores++, threads += (ptr->Processor.Flags == LTP_PC_SMT) ? 2 : 1;

        assert(ptr->Size);
        byteOffset += ptr->Size;
        ptr = (SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX*)(((char*)ptr) + ptr->Size);
    }

    free(buffer); // Cleanup

    // Run as many threads as possible on the same node until
    // core limit is reached, then move on filling the next node.
    for (int n = 0; n < nodes; n++)
        for (int i = 0; i < cores / nodes; i++)
            groups[groupSize++] = n;

    // In case a core has more than one logical processor (we assume 2) and we
    // still have threads to allocate, then spread them across available nodes.
    for (int t = 0; t < threads - cores; t++)
        groups[groupSize++] = t % nodes;

    // If we still have more threads than the total number of logical
    // processors then return -1 and let the OS to decide what to do.
    return index < groupSize ? groups[index] : -1;
}

void bindThisThread(int index) {

    // bindThisThread() sets the group affinity of the current thread

    GROUP_AFFINITY affinity;
    int group = bestGroup(index);

    // Check for a need to bind the thread
    if (group == -1) return;

    // Early exit if the needed APIs are not available at runtime
    HMODULE k32 = GetModuleHandle("Kernel32.dll");
    fun2_t fun2 = (fun2_t)GetProcAddress(k32, "GetNumaNodeProcessorMaskEx");
    fun3_t fun3 = (fun3_t)GetProcAddress(k32, "SetThreadGroupAffinity");
    if (!fun2 || !fun3) return;

    // Set the Affinity
    if (fun2(group, &affinity))
        fun3(GetCurrentThread(), &affinity, NULL);
}

#endif