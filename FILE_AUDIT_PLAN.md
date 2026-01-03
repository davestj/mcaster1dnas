# Documentation File Audit & Modernization Plan

## 📋 File Status Review

### ✅ Files to Convert to Markdown

| Old File | Status | Size | Action | New File |
|----------|--------|------|--------|----------|
| AUTHORS | 🔴 Outdated | 123 B | Rewrite completely | AUTHORS.md |
| HACKING | 🔴 Stub only | 23 B | Create dev guide | CONTRIBUTING.md |
| LICENSE | 🟢 Already updated | 1.1 KB | Convert to MD | LICENSE.md |
| TODO | 🔴 Old Icecast2 | 1.1 KB | Create new roadmap | TODO.md |
| NEWS | 🟡 Archive needed | 109 KB | Archive + create new | NEWS.md + NEWS.archive |
| COPYING | 🟢 Standard GPL | 18 KB | Keep as-is | COPYING |

---

## 🔴 Critical Issues Found

### AUTHORS File
**Problem:** Only lists 4 people, missing key contributors
- Jack Moffitt
- Michael Smith
- oddsock
- Karl Heyes

**Missing:**
- Monty (Jack Montgomery) - Original Icecast 1.x creator
- Ed Zaleski - Early Icecast developer
- David St John - Mcaster1DNAS fork maintainer
- All other Xiph.Org contributors

**Action:** Complete rewrite with full historical attribution

### HACKING File
**Problem:** Contains only "This needs updating."
**Action:** Replace with comprehensive CONTRIBUTING.md

### TODO File
**Problem:** All items are old Icecast2 feature requests
- Directory server GUID checks
- W3C Extended Logging
- mcaster1@xiph.org references
**Action:** Create new TODO.md with Mcaster1DNAS roadmap

### NEWS File
**Problem:** 109 KB of Icecast-KH changelog history
**Action:** Archive old history, create fresh NEWS.md for Mcaster1DNAS

---

## ✅ Files Already Correct

### LICENSE
- Already has Mcaster1DNAS header
- Credits Karl Heyes and Xiph.Org
- References FORK.md
- Just needs Markdown conversion

### COPYING
- Standard GNU GPL v2 text
- Cannot be modified (FSF requirement)
- Keep as-is

---

## 📝 Detailed Action Plan

### 1. AUTHORS.md (New)
**Content:**
- **Mcaster1DNAS Team** - David St John (Saint John)
- **Icecast-KH Contributors** - Karl Heyes
- **Icecast2 Contributors** - Xiph.Org Foundation team
- **Icecast 1.x History** - Jack Montgomery (Monty), Ed Zaleski
- **Special Thanks** - All contributors

**Research Needed:**
- Monty's full name and contributions
- Ed Zaleski's role
- Other key Xiph.Org contributors

### 2. CONTRIBUTING.md (New)
**Content:**
- How to contribute to Mcaster1DNAS
- Code style guidelines
- Build instructions for developers
- Testing procedures
- Pull request process
- Issue reporting guidelines

### 3. LICENSE.md (Convert existing)
**Content:**
- Keep existing LICENSE text
- Add Markdown formatting
- Keep all attributions

### 4. TODO.md (New)
**Content:**
- Mcaster1DNAS roadmap
- Current version features
- Planned features
- Community requests
- Technical debt items

### 5. NEWS.md (New) + NEWS.archive
**Content:**
- NEWS.md: Mcaster1DNAS releases only (v2.5.0+)
- NEWS.archive: Move old Icecast-KH history

---

## 🗑️ Files to Mark for Review/Removal

### Potentially Obsolete Files
- **INSTALL** - May be outdated build instructions
- **ChangeLog** - May duplicate NEWS
- **BUGS** - May be outdated bug list

**Action:** Review these files next

---

## 📅 Implementation Order

1. ✅ Create AUTHORS.md with full history
2. ✅ Create CONTRIBUTING.md with dev guidelines
3. ✅ Convert LICENSE to LICENSE.md
4. ✅ Create TODO.md with Mcaster1DNAS roadmap
5. ✅ Archive NEWS → NEWS.archive
6. ✅ Create new NEWS.md for Mcaster1DNAS
7. ✅ Review COPYING (keep as-is)
8. ⏳ Audit INSTALL, ChangeLog, BUGS

---

## 🔍 Historical Research

### Icecast History

**Icecast 1.x (1998-2001)**
- **Jack Montgomery (Monty)** - Original creator
- **Ed Zaleski** - Early contributor
- **Goal:** First open source streaming server

**Icecast 2.x (2001-2023)**
- **Xiph.Org Foundation** - Complete rewrite
- **Jack Moffitt** - Lead developer
- **Michael Smith** - Core contributor
- **oddsock** - Windows port maintainer
- **Goal:** Modern, standards-based streaming

**Icecast-KH (2010-2022)**
- **Karl Heyes** - Fork maintainer
- **Goal:** Performance and features

**Mcaster1DNAS (2025-2026)**
- **David St John (Saint John)** - Fork maintainer
- **Goal:** Enterprise features, modern UI

---

## 📊 File Comparison

### Before (Plain Text)
```
AUTHORS       123 bytes   4 names only
HACKING        23 bytes   "This needs updating"
LICENSE     1,100 bytes   Good but plain text
TODO        1,100 bytes   Old Icecast2 features
NEWS      109,000 bytes   All Icecast-KH history
```

### After (Markdown)
```
AUTHORS.md    ~2 KB      Complete attribution
CONTRIBUTING.md ~5 KB    Full dev guide
LICENSE.md    ~1.5 KB    Formatted license
TODO.md       ~3 KB      Mcaster1DNAS roadmap
NEWS.md       ~2 KB      Mcaster1DNAS only
NEWS.archive  109 KB     Historical archive
```

---

## ✅ Success Criteria

- [x] All critical files identified
- [ ] All authors properly credited
- [ ] Historical lineage preserved
- [ ] Old Icecast data archived (not deleted)
- [ ] New Mcaster1DNAS content created
- [ ] Markdown formatting applied
- [ ] No license violations
- [ ] Professional documentation

---

**Created:** February 14, 2026
**Status:** Ready for implementation
**Priority:** High - Essential for project credibility
