#!bin/bash
cd ../bin/testbed/

echo "=============================================="
# valgrind --leak-check=yes --suppressions=./vulkan.supp testbed
valgrind \
    --leak-check=full \
    --show-leak-kinds=all \
    --track-origins=yes \
    --gen-suppressions=all \
	./testbed
echo "[BUILDER]: Launching testbed with address sanitizer..."
echo "=============================================="
echo ""
