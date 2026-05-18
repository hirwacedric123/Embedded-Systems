#!/bin/bash
# Build Word document from submission report
set -e
cd "$(dirname "$0")"

pandoc SUBMISSION_REPORT.md -o SUBMISSION_REPORT.docx \
  --from markdown+yaml_metadata_block \
  --resource-path=".:figures" \
  --toc \
  --toc-depth=2 \
  --number-sections \
  --standalone

echo "Created: $(pwd)/SUBMISSION_REPORT.docx"
ls -lh SUBMISSION_REPORT.docx
