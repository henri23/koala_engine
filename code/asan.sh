#!bin/bash
cd ../bin/testbed/

echo "=============================================="
valgrind --leak-check=yes testbed
echo "[BUILDER]: Launching testbed with address sanitizer..."
echo "=============================================="
echo ""
