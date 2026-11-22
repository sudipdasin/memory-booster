# Security

MemBoost Pro is intended to run without elevated privileges except for explicit temporary kernel tuning. Do not run the GUI as root. Report vulnerabilities privately to the repository owner through GitHub's private vulnerability reporting if available, or a secure channel chosen by the owner. Avoid public issue comments containing tokens, usernames, running process names, system logs or confidential hardware inventories.

Current support is limited to the latest tested release; older downloads remain archived for diagnosis only. No automatic kernel writes, OOM killing, swap cycling or cache dropping are implemented. The included test suite never writes to the host `/proc/sys`.
