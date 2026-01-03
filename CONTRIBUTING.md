# Contributing to Mcaster1DNAS

Thank you for your interest in contributing to Mcaster1DNAS! This document provides guidelines for contributing to the project.

---

## 📋 Table of Contents

- [Code of Conduct](#code-of-conduct)
- [Getting Started](#getting-started)
- [Development Setup](#development-setup)
- [How to Contribute](#how-to-contribute)
- [Coding Standards](#coding-standards)
- [Testing](#testing)
- [Submitting Changes](#submitting-changes)
- [Documentation](#documentation)
- [Community](#community)

---

## 🤝 Code of Conduct

### Our Standards

- **Be respectful** - Treat all contributors with respect
- **Be collaborative** - Work together towards common goals
- **Be open** - Welcome newcomers and help them get started
- **Give credit** - Acknowledge others' contributions
- **Focus on quality** - Maintain high standards for code and documentation

### Unacceptable Behavior

- Harassment or discrimination
- Trolling or inflammatory comments
- Personal attacks
- Publishing private information
- Spam or off-topic discussions

---

## 🚀 Getting Started

### Prerequisites

Before contributing, ensure you have:

- **C development environment** (GCC 4.8+ or Clang 3.5+)
- **Autotools** (autoconf, automake, libtool)
- **Dependencies** (libxml2, libxslt, libcurl, OpenSSL)
- **Git** for version control
- **A GitHub account** for pull requests

### First-Time Contributors

New to open source? Start here:

1. **Read the documentation** - Familiarize yourself with Mcaster1DNAS
2. **Look for "good first issue" labels** - Start with beginner-friendly tasks
3. **Ask questions** - Use GitHub Discussions for help
4. **Start small** - Fix typos, improve docs, add tests

---

## 💻 Development Setup

### 1. Fork and Clone

```bash
# Fork the repository on GitHub, then:
git clone https://github.com/YOUR_USERNAME/mcaster1dnas.git
cd mcaster1dnas

# Add upstream remote
git remote add upstream https://github.com/davestj/mcaster1dnas.git
```

### 2. Install Dependencies

**Debian/Ubuntu:**
```bash
sudo apt-get update
sudo apt-get install build-essential autoconf automake libtool \
  libxml2-dev libxslt1-dev libcurl4-openssl-dev libssl-dev \
  libvorbis-dev libogg-dev libtheora-dev libspeex-dev
```

**CentOS/RHEL:**
```bash
sudo yum groupinstall "Development Tools"
sudo yum install autoconf automake libtool libxml2-devel \
  libxslt-devel libcurl-devel openssl-devel \
  libvorbis-devel libogg-devel libtheora-devel speex-devel
```

**macOS:**
```bash
brew install autoconf automake libtool libxml2 libxslt curl openssl \
  libogg libvorbis theora speex
```

### 3. Build from Source

```bash
# Generate build system
./autogen.sh

# Configure
./configure --prefix=/usr/local/mcaster1dnas \
  --with-openssl --with-ogg --with-vorbis

# Compile
make -j$(nproc)

# Run tests (if available)
make check

# Install locally for testing
sudo make install
```

### 4. Create a Branch

```bash
# Update main branch
git checkout main
git pull upstream main

# Create feature branch
git checkout -b feature/your-feature-name
```

---

## 📝 How to Contribute

### Types of Contributions

1. **Bug Fixes** - Fix reported bugs
2. **Features** - Add new functionality
3. **Documentation** - Improve docs, add examples
4. **Testing** - Write tests, improve coverage
5. **Refactoring** - Improve code quality
6. **UI/UX** - Enhance web interface
7. **Performance** - Optimize code
8. **Build System** - Improve build process

### Contribution Workflow

1. **Find or create an issue** - Discuss changes before coding
2. **Create a branch** - Use descriptive names (feature/*, bugfix/*, docs/*)
3. **Write code** - Follow coding standards
4. **Test thoroughly** - Ensure nothing breaks
5. **Commit changes** - Use clear, descriptive messages
6. **Push to fork** - Push your branch to GitHub
7. **Open pull request** - Describe your changes
8. **Address feedback** - Respond to review comments
9. **Get merged** - Celebrate your contribution! 🎉

---

## 🎨 Coding Standards

### C Code Style

**General Rules:**
- **Indentation:** Tabs (width: 4 spaces)
- **Line length:** Maximum 120 characters
- **Naming:** snake_case for functions and variables
- **Braces:** K&R style (opening brace on same line)

**Example:**
```c
/* Function documentation */
int process_client_request(client_t *client, const char *request) {
    if (client == NULL || request == NULL) {
        return -1;
    }

    // Process request
    int result = handle_request(client, request);
    if (result < 0) {
        ERROR("Failed to process request");
        return -1;
    }

    return 0;
}
```

**Naming Conventions:**
- Functions: `verb_noun()` (e.g., `create_client()`, `parse_config()`)
- Variables: Descriptive names (e.g., `client_count`, `buffer_size`)
- Constants: UPPERCASE with underscores (e.g., `MAX_CLIENTS`)
- Structs: `name_t` suffix (e.g., `client_t`, `source_t`)

**Comments:**
- Use `/* */` for multi-line comments
- Use `//` for single-line comments
- Document all public functions
- Explain complex logic
- Avoid obvious comments

### HTML/CSS/JavaScript Style

**HTML:**
- Use semantic HTML5 tags
- Indent with 4 spaces
- Use lowercase for tags and attributes
- Close all tags

**CSS:**
- Use CSS variables for colors
- Group related properties
- Use kebab-case for class names
- Comment major sections

**JavaScript:**
- Use `const` and `let`, never `var`
- Use semicolons
- Use template literals for strings
- Document functions with JSDoc

### XML Configuration

- Use 4-space indentation
- Keep element nesting clear
- Add comments for complex configurations

---

## 🧪 Testing

### Manual Testing

Before submitting:

1. **Build from clean state:**
   ```bash
   make clean
   ./autogen.sh && ./configure && make
   ```

2. **Test basic functionality:**
   - Start server with test config
   - Connect with a source client
   - Connect with a listener
   - Access admin interface
   - Check logs for errors

3. **Test your changes:**
   - Verify the specific feature/fix works
   - Test edge cases
   - Check for regressions

### Automated Testing

```bash
# Run test suite (if available)
make check

# Run specific tests
make test-NAME
```

### Testing Checklist

- [ ] Code compiles without warnings
- [ ] Server starts and stops cleanly
- [ ] No memory leaks (run with valgrind)
- [ ] Log files show expected output
- [ ] Admin interface loads correctly
- [ ] Streaming works (source → server → listener)
- [ ] SSL/HTTPS functions properly
- [ ] Configuration changes take effect

---

## 📤 Submitting Changes

### Commit Messages

**Format:**
```
Short summary (50 chars or less)

Longer explanation if needed (wrap at 72 chars).
Explain the "why" not the "what".

Fixes #123
```

**Examples:**

✅ **Good:**
```
Fix memory leak in client connection cleanup

The client struct wasn't being freed when connections closed
unexpectedly, causing gradual memory exhaustion under high load.

Fixes #456
```

❌ **Bad:**
```
fixed bug
```

### Pull Request Guidelines

**Title:** Clear, descriptive summary
**Description:** Include:
- What changes were made
- Why the changes were made
- How to test the changes
- Any breaking changes
- Related issues

**Example:**
```markdown
## Summary
Adds live clock display to admin interface header

## Changes
- Created mcaster-utils.js for clock functionality
- Updated all admin XSL pages with clock HTML
- Added CSS for clock styling
- Clock updates every second via JavaScript

## Testing
1. Start server
2. Open admin interface
3. Verify clock displays and updates

## Related Issues
Closes #789
```

### Before Submitting PR

- [ ] Code follows style guidelines
- [ ] Comments added for complex code
- [ ] Documentation updated if needed
- [ ] Tested on at least one platform
- [ ] No compiler warnings
- [ ] Commit messages are clear
- [ ] Branch is up to date with main

---

## 📚 Documentation

### What to Document

- **New features** - How to use them
- **Configuration changes** - New options
- **API changes** - Updated interfaces
- **Bug fixes** - What was fixed
- **Breaking changes** - Migration guide

### Documentation Locations

- **README.md** - Project overview
- **doc/** - Detailed documentation
- **Comments** - Code documentation
- **CHANGELOG** - Version history
- **Wiki** - User guides and tutorials

### Writing Guidelines

- **Be clear** - Use simple language
- **Be complete** - Include all necessary info
- **Be concise** - Avoid unnecessary words
- **Use examples** - Show, don't just tell
- **Keep updated** - Update docs with code changes

---

## 🌐 Community

### Getting Help

- **GitHub Issues** - Bug reports and feature requests
- **GitHub Discussions** - Questions and community help
- **Email** - davestj@gmail.com for direct contact

### Reporting Bugs

Include:
1. **Mcaster1DNAS version**
2. **Operating system and version**
3. **Steps to reproduce**
4. **Expected behavior**
5. **Actual behavior**
6. **Relevant logs** (with sensitive info redacted)
7. **Configuration** (anonymized if needed)

### Suggesting Features

Include:
1. **Use case** - What problem does it solve?
2. **Proposed solution** - How should it work?
3. **Alternatives** - Other ways to solve it?
4. **Impact** - Who would benefit?

---

## 🏆 Recognition

### Contributor Credits

All contributors are listed in:
- **AUTHORS.md** - Permanent credit
- **Release notes** - Feature announcements
- **Commit history** - Preserved forever

### Types of Recognition

- Listed in AUTHORS.md
- Mentioned in release notes
- Thanked in documentation
- Social media shoutouts
- Maintainer status (for long-term contributors)

---

## 📜 License

By contributing to Mcaster1DNAS, you agree that your contributions will be licensed under the GNU General Public License v2.

Your contributions:
- Must be your original work
- Must be compatible with GPL v2
- Become part of the project under GPL v2
- Can be used by anyone under GPL v2 terms

---

## ❓ FAQ

### Can I contribute if I'm new to C?

Yes! Start with:
- Documentation improvements
- Testing
- Bug reports
- UI/CSS improvements
- Translation

### How long does review take?

Usually 1-7 days, depending on complexity and maintainer availability.

### Can I contribute anonymously?

Yes, use any name/handle you prefer. Email is optional.

### What if my PR is rejected?

Don't be discouraged! Ask for feedback and try again. Sometimes:
- Feature doesn't fit project direction
- Code needs refactoring
- Better approach exists

Learn and improve!

### Can I work on multiple issues?

Yes, but finish one before starting another. Keep PRs focused.

---

## 🎯 Good First Issues

Look for issues labeled:
- `good first issue` - Beginner-friendly
- `documentation` - Improve docs
- `help wanted` - Maintainers need assistance
- `bug` - Fix broken functionality

---

## 📞 Contact

- **Maintainer:** David St John (Saint John)
- **Email:** davestj@gmail.com
- **GitHub:** [@davestj](https://github.com/davestj)
- **Website:** https://mcaster1.com

---

**Thank you for contributing to Mcaster1DNAS!** 🎉

Your contributions help make open source streaming better for everyone.

---

**Last Updated:** February 14, 2026
**Mcaster1DNAS Version:** 2.5.0
