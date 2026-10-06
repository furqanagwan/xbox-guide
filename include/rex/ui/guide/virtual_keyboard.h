/**
 * @file        rex/ui/guide/virtual_keyboard.h
 * @brief       The console's on-screen keyboard: layouts and editing (RG-GDK-059)
 *
 * @copyright   Copyright (c) 2026 Tom Clay
 * @license     BSD 3-Clause License
 */

#pragma once

#include <cstddef>
#include <string>
#include <string_view>

namespace rex::ui::guide {

/// The text a title asked for through XamShowKeyboardUI, edited as on the
/// console's keyboard (vk.xex): a 5 x 10 grid of keys on one of three pages
/// (the English layouts of the 2.0.17559 keyboard: QWERTY letters, symbols,
/// accented letters), Caps, a cursor, Backspace and Space.
class VirtualKeyboard {
 public:
  static constexpr int kRows = 5;
  static constexpr int kColumns = 10;

  enum class Page { kAlphabet, kSymbols, kAccents };
  static constexpr int kPages = 3;

  /// `max_length` characters at most (the guest buffer less its terminator).
  VirtualKeyboard(std::u16string text, size_t max_length);

  /// The character the key at `row`, `column` types now (Caps applied);
  /// 0 for no key.
  char16_t KeyAt(int row, int column) const;
  static std::string_view PageName(Page page);
  Page page() const { return page_; }
  /// The page the previous (LT) and next (RT) page keys go to.
  Page PreviousPage() const { return Page((int(page_) + kPages - 1) % kPages); }
  Page NextPage() const { return Page((int(page_) + 1) % kPages); }
  bool caps() const { return caps_; }

  /// Types the key at `row`, `column`; false when it has no character or the
  /// text is full.
  bool Type(int row, int column);
  bool Insert(char16_t c);
  bool Backspace();
  bool Space() { return Insert(u' '); }
  /// Moves the cursor by `delta` characters, within the text.
  void MoveCursor(int delta);
  void ToggleCaps() { caps_ = !caps_; }
  void GoToPage(Page page) { page_ = page; }

  const std::u16string& text() const { return text_; }
  size_t cursor() const { return cursor_; }
  size_t max_length() const { return max_length_; }

 private:
  std::u16string text_;
  size_t max_length_;
  size_t cursor_;
  Page page_ = Page::kAlphabet;
  bool caps_ = false;
};

}  // namespace rex::ui::guide
