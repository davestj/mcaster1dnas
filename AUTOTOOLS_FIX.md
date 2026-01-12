# Autotools Auto-Generation Fix
**Date:** February 15, 2026
**Issue:** README, AUTHORS, COPYING, NEWS, TODO being auto-generated as stubs

## Problem

When running `./autogen.sh`, automake was automatically creating empty stub versions of standard GNU files:
- README
- AUTHORS
- COPYING
- NEWS
- TODO
- ChangeLog
- INSTALL

This conflicted with our actual documentation files (README.md, TODO.md, etc.)

## Root Cause

Two issues in the build system:

1. **autogen.sh Line 102**: `$AUTOMAKE --add-missing --copy`
   - The `--add-missing` flag auto-creates missing "standard" files
   - Without `--foreign`, automake enforces GNU project standards

2. **Makefile.am Line 20**: `doc_DATA = README AUTHORS COPYING NEWS TODO`
   - Referenced non-existent files
   - Automake creates stubs when these files are missing

## Solution

### 1. Updated autogen.sh

**Before:**
```bash
$AUTOMAKE --add-missing --copy
```

**After:**
```bash
$AUTOMAKE --add-missing --copy --foreign
```

The `--foreign` flag tells automake not to enforce GNU standards, preventing automatic stub file creation.

### 2. Updated Makefile.am

**Added AUTOMAKE_OPTIONS:**
```makefile
AUTOMAKE_OPTIONS = foreign
```

This makes the foreign mode permanent across all automake runs.

**Updated doc_DATA:**
```makefile
# Before:
doc_DATA = README AUTHORS COPYING NEWS TODO

# After:
doc_DATA = README.md ChangeLog TODO.md \
	ICY2_PROTOCOL_SPEC.md YAML_IMPLEMENTATION.md YP_LOGGING_FEATURE.md \
	FORK.md BUILD_AND_RUN.md
```

Now references our actual .md documentation files instead of stub files.

## Testing

```bash
# Remove any existing stub files
rm -f README NEWS AUTHORS TODO COPYING ChangeLog INSTALL

# Run autogen
./autogen.sh

# Verify no stubs were created
ls README NEWS AUTHORS TODO COPYING INSTALL 2>&1
# Result: ✅ No auto-generated stub files!
```

## Installation

Documentation files now installed to `$(datadir)/doc/mcaster1dnas/`:
- README.md
- ChangeLog
- TODO.md
- ICY2_PROTOCOL_SPEC.md
- YAML_IMPLEMENTATION.md
- YP_LOGGING_FEATURE.md
- FORK.md
- BUILD_AND_RUN.md

## Benefits

1. **No more conflicts** - Our .md files won't be overwritten
2. **Clean git status** - No untracked stub files
3. **Proper documentation** - Real docs installed, not empty stubs
4. **GNU compatibility** - Still uses `--add-missing` for truly missing files (like Makefile.in)

## What `--foreign` Does

The `--foreign` flag relaxes automake's strictness by:
- Not requiring NEWS, README, AUTHORS, ChangeLog files
- Not enforcing GNU Coding Standards structure
- Allowing project-specific organization
- Still creating necessary build infrastructure (Makefile.in, etc.)

## Files Modified

1. **autogen.sh** - Added `--foreign` flag to automake
2. **Makefile.am** - Added `AUTOMAKE_OPTIONS = foreign` and updated `doc_DATA`

## Verification

After these changes:
```bash
./autogen.sh  # No stub files created ✅
./configure   # Works normally ✅
make          # Compiles correctly ✅
make install  # Installs our .md files ✅
```

## Related Documentation

- GNU Automake Manual: https://www.gnu.org/software/automake/manual/
- Section 3.1: "Strictness" - Explains foreign mode
- Section 14: "What Gets Installed" - doc_DATA behavior

---

**Status:** ✅ Fixed
**Version:** 2.6.0
**Impact:** Low (build system only, no runtime changes)
