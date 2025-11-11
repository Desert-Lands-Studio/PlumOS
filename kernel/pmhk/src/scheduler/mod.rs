use alloc::collections::BTreeMap;
use alloc::collections::VecDeque;
use spin::Mutex;

#[derive(Debug, Clone, Copy, PartialEq)]
pub enum ThreadState {
    Ready,
    Running,
    Blocked,
    Terminated,
}

#[derive(Debug, Clone)]
pub struct Thread {
    pub id: u64,
    pub state: ThreadState,
    pub stack_pointer: usize,
    pub instruction_pointer: usize,
}

pub struct Scheduler {
    threads: BTreeMap<u64, Thread>,
    ready_queue: VecDeque<u64>,
    current_thread: Option<u64>,
}

impl Scheduler {
    pub const fn new() -> Self {
        Self {
            threads: BTreeMap::new(),
            ready_queue: VecDeque::new(),
            current_thread: None,
        }
    }

    pub fn add_thread(&mut self, thread: Thread) {
        let thread_id = thread.id;
        self.threads.insert(thread_id, thread);
        self.ready_queue.push_back(thread_id);
    }

    pub fn schedule(&mut self) -> Option<Thread> {
        if let Some(current_id) = self.current_thread {
            if let Some(thread) = self.threads.get_mut(&current_id) {
                thread.state = ThreadState::Ready;
            }
            self.ready_queue.push_back(current_id);
        }

        let mut next_id = None;
        while let Some(candidate_id) = self.ready_queue.pop_front() {
            if let Some(thread) = self.threads.get(&candidate_id) {
                if thread.state == ThreadState::Ready {
                    next_id = Some(candidate_id);
                    break;
                }
            }
        }

        if let Some(id) = next_id {
            if let Some(thread) = self.threads.get_mut(&id) {
                thread.state = ThreadState::Running;
                self.current_thread = Some(id);
                return Some(thread.clone());
            }
        }
        None
    }
}

static SCHEDULER: Mutex<Scheduler> = Mutex::new(Scheduler::new());

pub fn init() {
}

pub fn add_thread(thread: Thread) {
    SCHEDULER.lock().add_thread(thread);
}

pub fn schedule() -> Option<Thread> {
    SCHEDULER.lock().schedule()
}