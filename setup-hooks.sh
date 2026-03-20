#!/bin/sh
# Install Git hooks.

mkdir -p .githooks
git config core.hooksPath .githooks
chmod +x .githooks/pre-commit
echo "Git hooks installed! Run 'git commit' to test."
echo ""
echo "To disable: git config core.hooksPath false"
