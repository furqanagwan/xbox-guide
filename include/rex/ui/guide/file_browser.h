// Copyright (c) 2026 Xbox Guide contributors. BSD 3-Clause License; see LICENSE.
#pragma once

#include <filesystem>
#include <functional>
#include <string>
#include <vector>

#include <rex/ui/guide/guide_list_page.h>

namespace rex::ui::guide {

// Drives, folders and files as rows for a GuideListPage: the console's file
// pickers had no Windows dialog, so the Guide browses the PC itself. The model
// only lists and navigates; the host checks whatever is chosen. Hidden and
// system entries are left out, folders come first, then matching files with
// their sizes; each row's details are its full path.
class GuideFileBrowser {
 public:
  enum class Pick { kFile, kFolder };
  struct Result {
    enum class Kind { kNone, kOpened, kChosen };
    Kind kind = Kind::kNone;
    std::filesystem::path path;  // kChosen: the file, or the folder to use
  };
  static constexpr size_t kMaxEntries = 4096;

  // `extensions`: the files kFile lists, lower case with the dot (".iso").
  GuideFileBrowser(Pick pick, std::vector<std::wstring> extensions = {});

  // Lists `folder`, or the drives when it is empty, with focus on `focus`
  // when it is listed.
  void Open(std::filesystem::path folder, const std::filesystem::path& focus = {});
  // A on `row`: opens a drive or folder, or chooses a file (kFile) or the
  // open folder's Use This Folder row (kFolder).
  Result Select(size_t row);
  // B: the parent folder, then the drives, focusing where it came from.
  // False at the drives, where B leaves the browser.
  bool Up();

  const std::filesystem::path& folder() const { return folder_; }
  const std::vector<GuideListRow>& rows() const { return rows_; }
  size_t focus() const { return focus_; }
  // The details pane with no rows.
  std::string empty_details() const;

  // The top level's drive roots; Open uses fixed, removable and network
  // drives unless a host or test replaces it.
  std::function<std::vector<std::filesystem::path>()> list_drives;

 private:
  bool use_folder_row() const { return pick_ == Pick::kFolder && !folder_.empty(); }

  Pick pick_;
  std::vector<std::wstring> extensions_;
  std::filesystem::path folder_;
  std::vector<std::filesystem::path> entries_;
  std::vector<GuideListRow> rows_;
  size_t focus_ = 0;
};

// "7.3 GB", "512.0 MB" or "12 KB", as the Guide shows sizes.
std::string FormatGuideSize(uint64_t bytes);

}  // namespace rex::ui::guide
