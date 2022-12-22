for any time t and current position p the (remaining) valves v can be expressed as functions f_v(x) = rate_v * (x - dist[p, v])

going from remaining time t = 0 towards 30 allows at each minute to record bounds for max flow from this point onwards

upperflow[p, t]
upperflow[v, 30] = 0
upperflow[v, 29] = rate_v
upperflow[v, 28] = rate_v * 2

when moving from a position u to v, the set of valves can be separated into the ones that are now further away and the ones that are at the same distance or closer.

With increasing distance from any valve v, more valves will become reachable.  For each distance 0,1,2,... the problem can be solved locally.