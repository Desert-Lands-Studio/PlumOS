use alloc::collections::BTreeMap;
use alloc::vec::Vec;

use spin::Mutex;

pub struct Message {
    pub sender: u64,
    pub recipient: u64,
    pub data: Vec<u8>,
}

pub struct Endpoint {
    pub messages: Vec<Message>,
    pub waiting_thread: Option<u64>,
}

pub struct IpcManager {
    endpoints: BTreeMap<u64, Endpoint>,
    next_port: u64,
}

impl IpcManager {
    pub const fn new() -> Self {
        Self {
            endpoints: BTreeMap::new(),
            next_port: 1,
        }
    }
    
    pub fn send(&mut self, recipient: u64, data: &[u8]) -> Result<(), &'static str> {
        let message = Message {
            sender: 0,
            recipient,
            data: data.to_vec(),
        };
        
        if let Some(endpoint) = self.endpoints.get_mut(&recipient) {
            endpoint.messages.push(message);
            Ok(())
        } else {
            Err("Endpoint not found")
        }
    }
    
    pub fn receive(&mut self, endpoint: u64) -> Option<Message> {
        if let Some(ep) = self.endpoints.get_mut(&endpoint) {
            if !ep.messages.is_empty() {
                Some(ep.messages.remove(0))
            } else {
                None
            }
        } else {
            None
        }
    }
}

static IPC_MANAGER: Mutex<IpcManager> = Mutex::new(IpcManager::new());

pub fn init() {
    // Инициализация IPC системы
}

pub fn send(recipient: u64, data: &[u8]) -> Result<(), &'static str> {
    IPC_MANAGER.lock().send(recipient, data)
}

pub fn receive(endpoint: u64) -> Option<Message> {
    IPC_MANAGER.lock().receive(endpoint)
}