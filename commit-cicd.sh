#!/bin/bash
# Git commit script for CI/CD and Version Management implementation
# Generated on: February 16, 2026

echo "=== Adding CI/CD and Version Management Files ==="
echo ""

# Add new core files
echo "Adding version management files..."
git add VERSION
git add scripts/version-bump.sh
git add GIT-VERSION-GEN

# Add CI/CD workflow files
echo "Adding GitHub Actions workflow..."
git add .github/workflows/development.yml
git add .github/workflows/README.md

# Add documentation
echo "Adding documentation files..."
git add CI_CD.md
git add VERSION_MANAGEMENT.md
git add CI_CD_SETUP_COMPLETE.md
git add PODCAST_PLANNING.md

# Add modified configuration files
echo "Adding modified build files..."
git add Makefile.am
git add configure.ac
git add .gitignore

# Add changelog and todo updates
echo "Adding updated project files..."
git add ChangeLog
git add TODO.md

# Show status
echo ""
echo "=== Files staged for commit ==="
git status --short | grep "^[AM]"

echo ""
echo "=== Ready to commit! ==="
echo ""
echo "Review the staged files above."
echo "If everything looks good, run the commit command below:"
echo ""
echo "Press Enter to see the commit command, or Ctrl+C to cancel..."
read

echo ""
echo "==============================================="
echo "Git Commit Command:"
echo "==============================================="
echo ""

cat << 'EOF'
git commit -m "feat: Implement CI/CD pipeline and automated version management

This commit implements a comprehensive CI/CD system and automated version
management for Mcaster1DNAS, enabling automated builds, testing, and releases.

## CI/CD Pipeline

- Add GitHub Actions workflow (.github/workflows/development.yml)
  - Automated lint checks (XSL, YAML, security)
  - Build on Debian 12 with all dependencies
  - HTTP server testing with curl
  - Admin authentication testing
  - YAML configuration validation
  - Versioned artifact creation
  - 7-day artifact retention

- Add workflow documentation (.github/workflows/README.md)
  - Job descriptions and test coverage
  - Dependency lists and versions
  - Local testing instructions
  - Troubleshooting guide

## Version Management System

- Add VERSION file as single source of truth (2.5.1)
- Update GIT-VERSION-GEN for branch-based versioning
  - main/master → Production (2.5.1)
  - rc/* → Release Candidate (2.5.1-rc1)
  - development → Beta (2.5.1-beta.YYYYMMDD.HHMMSS)
  - feature/* → Alpha (2.5.1-alpha.YYYYMMDD.HHMMSS)

- Add scripts/version-bump.sh for automated version bumping
  - Auto-increment major/minor/patch versions
  - Branch-aware version generation
  - Git tag creation support
  - Interactive prompts

- Update configure.ac to read version from VERSION file
  - m4_define macro for version extraction
  - Single source of truth for version number

- Add Makefile targets for version management
  - make version - Display version information
  - make version-bump-major/minor/patch - Increment version
  - make version-bump-auto - Branch-aware auto-bump

## Documentation

- Add CI_CD.md - Comprehensive CI/CD system guide
  - Build process explanation
  - Local testing with Docker
  - Dependency management
  - Artifact information

- Add VERSION_MANAGEMENT.md - Version system documentation
  - Semantic versioning format
  - Branch-based release types
  - Workflow examples and best practices
  - Troubleshooting guide

- Add CI_CD_SETUP_COMPLETE.md - Implementation summary
  - Complete file inventory
  - Usage examples
  - Testing verification
  - Next steps and success criteria

- Add PODCAST_PLANNING.md - Future podcast feature roadmap
  - 6-week implementation plan
  - RSS generation, web UI, live recording
  - Paused pending CI/CD validation

- Update ChangeLog with recent changes
  - Web player feature (2.5.1-rc1)
  - Metadata display fixes
  - Podcast planning announcement

- Update TODO.md with podcast features
  - Moved podcast tasks to v2.6.1
  - Added web player as completed
  - Updated version timeline

- Update .gitignore for CI/CD artifacts
  - install-root/, *.deb, *.tar.gz
  - test-logs/, ci-test-*

## Testing

- Version generation verified on development branch
  - Output: 2.5.1-beta.20260216.040625-modified
- Make targets tested and working
- Git version header generation confirmed

## Breaking Changes

None - All changes are additive and backward compatible

## Next Steps

1. Push to development branch to trigger CI/CD
2. Validate GitHub Actions workflow execution
3. Review build artifacts and test results
4. Begin podcast feature implementation (v2.6.1)
5. Create .deb packages for Debian/Ubuntu

Co-Authored-By: Claude Sonnet 4.5 <noreply@anthropic.com>"
EOF

echo ""
echo "==============================================="
echo ""
echo "Copy the command above and run it to commit all changes."
