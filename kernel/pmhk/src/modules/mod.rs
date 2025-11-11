use alloc::collections::BTreeMap;
use alloc::string::String;
use spin::Mutex;

pub type ModuleInitFn = extern "C" fn() -> i32;
pub type ModuleExitFn = extern "C" fn();

pub struct Module {
    pub name: String,
    pub init: Option<ModuleInitFn>,
    pub exit: Option<ModuleExitFn>,
    pub state: ModuleState,
}

#[derive(Debug, PartialEq)]
pub enum ModuleState {
    Unloaded,
    Loaded,
    Initialized,
    Running,
}

pub struct ModuleManager {
    modules: BTreeMap<String, Module>,
}

impl ModuleManager {
    pub const fn new() -> Self {
        Self {
            modules: BTreeMap::new(),
        }
    }

    pub fn load_plam_module(&mut self, name: &str, _data: &[u8]) -> Result<(), &'static str> {
        let module = Module {
            name: String::from(name),
            init: None,
            exit: None,
            state: ModuleState::Loaded,
        };
        self.modules.insert(String::from(name), module);
        Ok(())
    }

    pub fn init_module(&mut self, name: &str) -> Result<(), &'static str> {
        if let Some(module) = self.modules.get_mut(name) {
            module.state = ModuleState::Initialized;
            Ok(())
        } else {
            Err("Module not found")
        }
    }
}

static MODULE_MANAGER: Mutex<ModuleManager> = Mutex::new(ModuleManager::new());

pub fn load_plam_module(name: &str, data: &[u8]) -> Result<(), &'static str> {
    MODULE_MANAGER.lock().load_plam_module(name, data)
}

pub fn init_module(name: &str) -> Result<(), &'static str> {
    MODULE_MANAGER.lock().init_module(name)
}