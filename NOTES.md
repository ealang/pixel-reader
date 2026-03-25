# Pixel Reader Notes

## Project

Add lightweight ebook cover support to Pixel Reader for Onion on the Miyoo Mini+.

## Goal

Improve browsing books without turning the app into a heavy bookshelf UI.

## Current Understanding

- The file browser is currently a text-only list of filenames.
- EPUB metadata parsing currently handles manifest, spine, and TOC, but not title/author/cover metadata.
- The reader already supports rendering images from inside EPUB content.
- A lightweight cover preview is more realistic than a full gallery view.

## Likely MVP

- Extract cover metadata from EPUB files.
- Load the selected book's cover when a file is focused.
- Show a small preview pane or modal for the highlighted book.
- Cache decoded cover images so browsing stays responsive.

## Constraints

- Keep the UI simple and readable on a small screen.
- Avoid heavy parsing on every scroll event.
- Keep the change realistic for upstream contribution.

## Next Steps

1. Inspect EPUB metadata flow and identify where cover references should be parsed.
2. Inspect file browser rendering path and choose the smallest UI change that can display a cover.
3. Decide whether to parse embedded covers directly or support sidecar/cached cover images first.
4. Build a minimal prototype.
