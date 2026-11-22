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
struct Diagnosis {
 std::string level="unknown";
 std::vector<std::string> findings, advice;
};
struct Process { int pid=0; std::string name; uint64_t rss_kib=0; };
struct Snapshot { Memory memory; Pressure pressure; std::vector<Process> processes; std::vector<std::string> zram; Cgroup cgroup; std::string swappiness="unavailable", vfs_cache_pressure="unavailable"; };
struct Profile { std::string name; int swappiness; int vfs_cache_pressure; std::string explanation; };
struct ApplyResult { bool ok=false; std::string message; };
Memory parse_memory(std::istream& input);
Pressure parse_pressure(std::istream& input);
Cgroup parse_cgroup(const std::string& current,const std::string& limit,const std::string& swap_current,const std::string& swap_limit);
Diagnosis diagnose(const Snapshot& s);
std::string diagnostic_report(const Snapshot& s);
Snapshot collect();
std::vector<Profile> profiles();
ApplyResult apply_profile(const Profile& p, const std::string& proc_root="/proc", bool dry_run=false);
std::string report(const Snapshot& s);
std::string json(const Snapshot& s);
}
