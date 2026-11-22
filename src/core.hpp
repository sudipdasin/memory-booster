// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 MemBoost Pro contributors
#pragma once
#include <cstdint>
#include <istream>
#include <string>
#include <vector>

namespace mb {
struct Memory { uint64_t total=0, available=0, free=0, cached=0, buffers=0, swap_total=0, swap_free=0; bool valid=false; };
struct Pressure { double some10=0, some60=0, full10=0, full60=0; bool available=false; };
struct Cgroup {
 bool available=false, limited=false, swap_available=false, swap_limited=false;
 uint64_t current_bytes=0, limit_bytes=0, swap_current_bytes=0, swap_limit_bytes=0;
};
// Cumulative counters: never interpret these as instantaneous load.
struct VmCounters {
 bool available=false, oom_available=false;
 uint64_t major_faults=0, swap_in=0, swap_out=0, oom_kills=0;
};
struct CgroupEvents {
 bool available=false;
 uint64_t low=0, high=0, max=0, oom=0, oom_kill=0;
};
struct TelemetryDelta {
 bool vm_available=false, cgroup_available=false, reset_detected=false;
 double seconds=0;
 uint64_t major_faults=0, swap_in=0, swap_out=0, oom_kills=0;
 uint64_t cgroup_high=0, cgroup_oom=0, cgroup_oom_kill=0;
};
struct Diagnosis {
 std::string level="unknown";
 std::vector<std::string> findings, advice;
};
struct Process { int pid=0; std::string name; uint64_t rss_kib=0; };
struct Snapshot { Memory memory; Pressure pressure; std::vector<Process> processes; std::vector<std::string> zram; Cgroup cgroup; VmCounters vmstat; CgroupEvents cgroup_events; std::string swappiness="unavailable", vfs_cache_pressure="unavailable"; };
struct Profile { std::string name; int swappiness; int vfs_cache_pressure; std::string explanation; };
struct ApplyResult { bool ok=false; std::string message; };
// A one-click *preview*, not a promise of reclaimable RAM or speed improvement.
// Unsafe situations produce guidance rather than speculative kernel writes.
enum class OptimizationAction { observe, offer_responsive, investigate, unavailable };
struct OptimizationPlan {
 OptimizationAction action=OptimizationAction::unavailable;
 std::string headline, explanation;
 bool can_apply=false;
};
OptimizationPlan plan_optimization(const Snapshot& s);
std::string optimization_report(const Snapshot& s);

Memory parse_memory(std::istream& input);
Pressure parse_pressure(std::istream& input);
VmCounters parse_vmstat(std::istream& input);
CgroupEvents parse_cgroup_events(std::istream& input);
TelemetryDelta telemetry_delta(const Snapshot& before, const Snapshot& after, double seconds);
std::string signals_report(const Snapshot& s, const TelemetryDelta& diff);
std::string telemetry_json(const Snapshot& s, const TelemetryDelta& diff, unsigned sample);
Cgroup parse_cgroup(const std::string& current,const std::string& limit,const std::string& swap_current,const std::string& swap_limit);
Diagnosis diagnose(const Snapshot& s);
std::string diagnostic_report(const Snapshot& s);
Snapshot collect();
std::vector<Profile> profiles();
ApplyResult apply_profile(const Profile& p, const std::string& proc_root="/proc", bool dry_run=false);
std::string report(const Snapshot& s);
std::string json(const Snapshot& s);
}
