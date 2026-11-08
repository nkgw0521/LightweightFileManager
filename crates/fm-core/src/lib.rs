use serde::Serialize;
use std::{fs, io, path::Path, time::UNIX_EPOCH};

#[derive(Serialize)]
struct FileEntry {
    name: String,
    path: String,
    is_dir: bool,
    size: u64,
    modified: u64,
}

#[cxx::bridge]
mod ffi {
    extern "Rust" {
        fn read_directory(path: &str) -> String;
        fn create_directory(path: &str) -> String;
        fn rename_path(from: &str, to: &str) -> String;
        fn remove_path(path: &str) -> String;
        fn copy_path(from: &str, to: &str) -> String;
        fn move_path(from: &str, to: &str) -> String;
    }
}

fn read_directory(path: &str) -> String {
    let dir = Path::new(path);
    let mut entries: Vec<FileEntry> = match fs::read_dir(dir) {
        Ok(items) => items
            .filter_map(Result::ok)
            .map(|item| {
                let path = item.path();
                let metadata = item.metadata().ok();
                let is_dir = metadata.as_ref().is_some_and(|m| m.is_dir());
                let size = metadata.as_ref().map_or(0, |m| m.len());
                let modified = metadata
                    .and_then(|m| m.modified().ok())
                    .and_then(|t| t.duration_since(UNIX_EPOCH).ok())
                    .map_or(0, |d| d.as_secs());
                FileEntry {
                    name: item.file_name().to_string_lossy().into_owned(),
                    path: path.to_string_lossy().into_owned(),
                    is_dir,
                    size,
                    modified,
                }
            })
            .collect(),
        Err(_) => Vec::new(),
    };

    // Directories first, then case-insensitive names for a predictable listing.
    entries.sort_by(|a, b| {
        b.is_dir
            .cmp(&a.is_dir)
            .then_with(|| a.name.to_lowercase().cmp(&b.name.to_lowercase()))
    });
    serde_json::to_string(&entries).unwrap_or_else(|_| "[]".to_owned())
}

fn error_text(result: io::Result<()>) -> String {
    result.err().map_or_else(String::new, |error| error.to_string())
}

fn create_directory(path: &str) -> String {
    error_text(fs::create_dir(path))
}

fn rename_path(from: &str, to: &str) -> String {
    error_text(fs::rename(from, to))
}

fn remove_path(path: &str) -> String {
    let path = Path::new(path);
    let result = match fs::symlink_metadata(path) {
        Ok(metadata) if metadata.is_dir() && !metadata.file_type().is_symlink() => fs::remove_dir_all(path),
        Ok(_) => fs::remove_file(path),
        Err(error) => Err(error),
    };
    error_text(result)
}

fn copy_entry(from: &Path, to: &Path) -> io::Result<()> {
    let metadata = fs::symlink_metadata(from)?;
    if metadata.file_type().is_symlink() {
        return Err(io::Error::new(io::ErrorKind::Unsupported, "symbolic link copying is not supported"));
    }
    if metadata.is_dir() {
        fs::create_dir(to)?;
        for entry in fs::read_dir(from)? {
            let entry = entry?;
            copy_entry(&entry.path(), &to.join(entry.file_name()))?;
        }
        Ok(())
    } else {
        fs::copy(from, to).map(|_| ())
    }
}

fn copy_path(from: &str, to: &str) -> String {
    let destination = Path::new(to);
    if destination.exists() {
        return "destination already exists".to_owned();
    }
    error_text(copy_entry(Path::new(from), destination))
}

fn move_path(from: &str, to: &str) -> String {
    match fs::rename(from, to) {
        Ok(()) => String::new(),
        Err(rename_error) => {
            if let Err(copy_error) = copy_entry(Path::new(from), Path::new(to)) {
                return format!("{rename_error}; copy fallback failed: {copy_error}");
            }
            let remove_error = remove_path(from);
            if !remove_error.is_empty() {
                let _ = remove_path(to);
                return remove_error;
            }
            String::new()
        }
    }
}
