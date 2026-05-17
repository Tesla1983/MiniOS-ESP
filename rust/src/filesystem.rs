use std::collections::BTreeMap;

pub trait FileSystem {
    fn write_file(&mut self, name: &str, contents: &str) -> Result<(), String>;
    fn append_file(&mut self, name: &str, contents: &str) -> Result<(), String>;
    fn read_file(&self, name: &str) -> Result<String, String>;
    fn delete_file(&mut self, name: &str) -> Result<(), String>;
    fn rename_file(&mut self, old: &str, new: &str) -> Result<(), String>;
    fn copy_file(&mut self, source: &str, dest: &str) -> Result<(), String>;
    fn list_files(&self) -> Vec<(String, usize)>;
}

#[derive(Debug, Default, Clone)]
pub struct MemoryFileSystem {
    files: BTreeMap<String, String>,
}

impl MemoryFileSystem {
    pub fn new() -> Self {
        Self::default()
    }

    fn validate_name(name: &str) -> Result<String, String> {
        let trimmed = name.trim().trim_start_matches('/');
        if trimmed.is_empty() {
            return Err("filename must not be empty".to_string());
        }
        if trimmed.contains("..") {
            return Err("parent directory traversal is not supported".to_string());
        }
        Ok(trimmed.to_string())
    }
}

impl FileSystem for MemoryFileSystem {
    fn write_file(&mut self, name: &str, contents: &str) -> Result<(), String> {
        let name = Self::validate_name(name)?;
        self.files.insert(name, contents.to_string());
        Ok(())
    }

    fn append_file(&mut self, name: &str, contents: &str) -> Result<(), String> {
        let name = Self::validate_name(name)?;
        self.files.entry(name).or_default().push_str(contents);
        Ok(())
    }

    fn read_file(&self, name: &str) -> Result<String, String> {
        let name = Self::validate_name(name)?;
        self.files
            .get(&name)
            .cloned()
            .ok_or_else(|| format!("file not found: {name}"))
    }

    fn delete_file(&mut self, name: &str) -> Result<(), String> {
        let name = Self::validate_name(name)?;
        self.files
            .remove(&name)
            .map(|_| ())
            .ok_or_else(|| format!("file not found: {name}"))
    }

    fn rename_file(&mut self, old: &str, new: &str) -> Result<(), String> {
        let old = Self::validate_name(old)?;
        let new = Self::validate_name(new)?;
        let contents = self
            .files
            .remove(&old)
            .ok_or_else(|| format!("file not found: {old}"))?;
        self.files.insert(new, contents);
        Ok(())
    }

    fn copy_file(&mut self, source: &str, dest: &str) -> Result<(), String> {
        let source = Self::validate_name(source)?;
        let dest = Self::validate_name(dest)?;
        let contents = self
            .files
            .get(&source)
            .cloned()
            .ok_or_else(|| format!("file not found: {source}"))?;
        self.files.insert(dest, contents);
        Ok(())
    }

    fn list_files(&self) -> Vec<(String, usize)> {
        self.files
            .iter()
            .map(|(name, contents)| (name.clone(), contents.len()))
            .collect()
    }
}
