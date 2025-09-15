#!/usr/bin/env python3
"""
format-code.py - Wrapper for projet-alpha.

Calls the format-code.py from emeraude-engine with local configuration.
"""

import subprocess
import sys
from pathlib import Path

def main():
    # Path to the engine's format-code.py
    script_dir = Path(__file__).parent
    engine_script = script_dir / "dependencies" / "emeraude-engine" / "format-code.py"

    if not engine_script.exists():
        print(f"Error: Engine script not found at {engine_script}", file=sys.stderr)
        sys.exit(1)

    # Run with local config (format-code.json in current directory)
    result = subprocess.run(
        [sys.executable, str(engine_script)] + sys.argv[1:],
        cwd=script_dir
    )

    sys.exit(result.returncode)


if __name__ == "__main__":
    main()
