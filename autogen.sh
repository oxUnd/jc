#!/bin/sh

echo "Generating build system..."

# Create m4 directory if it doesn't exist
mkdir -p m4

# Run autoreconf
autoreconf --install --force

echo "Done. You can now run ./configure"
