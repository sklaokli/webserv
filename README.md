#webserv

## Routing component (Task 01)

The routing, static-file, and AutoIndex component is implemented in C++98.
Read the [Thai implementation guide](docs/TASK01_ROUTING.md) for design decisions,
security policy, root/alias examples, and team integration instructions.

On Linux/WSL2 with procfs available, run from the repository root:

    make

The experimental tests/ directory and conf/routing-demo.conf are local-only and ignored by Git.
The optional route-demo and test-router targets require those local files;
they are not available in a fresh clone. Earlier test results in the guide describe the local harness.
The main executable currently parses configuration only; it does not listen on a port.
Socket/event-loop integration, CGI execution, and upload/delete handlers remain separate tasks.

AI assisted with routing implementation, config integration, tests, and documentation.
See the guide for references, verified results, and limitations.
