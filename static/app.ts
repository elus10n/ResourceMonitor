// Интерфейсы, соответствующие JSON-структуре из data_manager.cpp
interface CoreData {
    name: string;
    percent: number;
}

interface CPUData {
    total_percent: number;
    cores: CoreData[];
}

interface MemData {
    total_kb: number;
    free_kb: number;
    available_kb: number;
    usage_percent: number;
}

interface ProcData {
    pid: number;
    owner: string;
    name: string;
    state: string;
    cpu_percent: number;
    mem_size_pages: number;
    mem_resident_pages: number;
    command: string;
}

interface ProcessCollection {
    count_of_processes: number;
    count_of_zombie: number;
    count_of_running: number;
    count_of_sleeping: number;
    processes: ProcData[];
}

interface SysData {
    cpu: CPUData;
    memory: MemData;
    processes: ProcessCollection;
}

// Глобальное состояние
let currentProcesses: ProcData[] = [];
let sortKey: keyof ProcData = 'pid';
let sortAsc: boolean = true;
let searchQuery: string = '';

// Размер страницы в Linux обычно 4 КБ
const PAGE_SIZE_KB = 4;

document.addEventListener('DOMContentLoaded', () => {
    initWebSocket();
    initTableControls();
});

function initWebSocket(): void {
    const protocol = window.location.protocol === 'https:' ? 'wss:' : 'ws:';
    const wsUrl = `${protocol}//${window.location.host}/ws`;
    const ws = new WebSocket(wsUrl);

    const statusBadge = document.getElementById('ws-status');

    ws.onopen = () => {
        if (statusBadge) {
            statusBadge.textContent = 'Connected';
            statusBadge.classList.add('connected');
        }
    };

    ws.onclose = () => {
        if (statusBadge) {
            statusBadge.textContent = 'Disconnected';
            statusBadge.classList.remove('connected');
        }
        // Опционально: попытка реконнекта через таймаут
        setTimeout(initWebSocket, 3000);
    };

    ws.onmessage = (event: MessageEvent) => {
        try {
            const data: SysData = JSON.parse(event.data);
            updateDashboard(data);
        } catch (e) {
            console.error('Failed to parse WebSocket message:', e);
        }
    };
}

function updateDashboard(data: SysData): void {
    updateGlobalStats(data.processes);
    updateCPU(data.cpu);
    updateMemory(data.memory);
    
    currentProcesses = data.processes.processes;
    renderTable();
}

function updateGlobalStats(procData: ProcessCollection): void {
    setTextContent('stat-proc-count', procData.count_of_processes.toString());
    setTextContent('stat-work-count', procData.count_of_running.toString());
    setTextContent('stat-sleep-count', procData.count_of_sleeping.toString());
    setTextContent('stat-zombie-count', procData.count_of_zombie.toString());
}

function updateCPU(cpuData: CPUData): void {
    const totalPercent = cpuData.total_percent.toFixed(1);
    setTextContent('cpu-total-val', totalPercent);
    
    const totalBar = document.getElementById('cpu-total-bar');
    if (totalBar) totalBar.style.width = `${totalPercent}%`;

    const coresContainer = document.getElementById('cpu-cores-container');
    if (coresContainer) {
        coresContainer.innerHTML = '';
        cpuData.cores.forEach(core => {
            const percent = core.percent.toFixed(1);
            
            const coreDiv = document.createElement('div');
            coreDiv.className = 'cpu-core';
            coreDiv.innerHTML = `
                <div class="core-label">${core.name}: ${percent}%</div>
                <div class="progress-bar-container mini-progress">
                    <div class="progress-bar" style="width: ${percent}%"></div>
                </div>
            `;
            coresContainer.appendChild(coreDiv);
        });
    }
}

function updateMemory(memData: MemData): void {
    const totalMB = Math.round(memData.total_kb / 1024);
    const freeMB = Math.round(memData.free_kb / 1024);
    const availMB = Math.round(memData.available_kb / 1024);
    const usedMB = totalMB - availMB;
    const usagePercent = memData.usage_percent.toFixed(1);

    setTextContent('mem-total-val', totalMB.toString());
    setTextContent('mem-free-val', freeMB.toString());
    setTextContent('mem-avail-val', availMB.toString());
    setTextContent('mem-used-val', usedMB.toString());
    setTextContent('mem-percent-val', usagePercent);

    const memBar = document.getElementById('mem-usage-bar');
    if (memBar) memBar.style.width = `${usagePercent}%`;
}

function initTableControls(): void {
    // Настройка поиска
    const searchInput = document.getElementById('proc-search') as HTMLInputElement;
    if (searchInput) {
        searchInput.addEventListener('input', (e) => {
            searchQuery = (e.target as HTMLInputElement).value.toLowerCase();
            renderTable();
        });
    }

    // Настройка сортировки по заголовкам
    const headers = document.querySelectorAll('th.sortable');
    headers.forEach(th => {
        th.addEventListener('click', () => {
            const key = th.getAttribute('data-sort') as keyof ProcData;
            
            if (sortKey === key) {
                sortAsc = !sortAsc; // Меняем направление
            } else {
                sortKey = key;
                sortAsc = true;     // По умолчанию по возрастанию для новой колонки
            }

            // Обновляем UI классов сортировки
            headers.forEach(h => {
                h.classList.remove('sorted-asc', 'sorted-desc');
            });
            th.classList.add(sortAsc ? 'sorted-asc' : 'sorted-desc');

            renderTable();
        });
    });
}

function renderTable(): void {
    const tbody = document.getElementById('proc-table-body');
    if (!tbody) return;

    // 1. Фильтрация
    let filteredProcs = currentProcesses;
    if (searchQuery) {
        filteredProcs = filteredProcs.filter(p => 
            p.name.toLowerCase().includes(searchQuery) || 
            p.command.toLowerCase().includes(searchQuery) ||
            p.owner.toLowerCase().includes(searchQuery) ||
            p.pid.toString().includes(searchQuery)
        );
    }

    // 2. Сортировка
    filteredProcs.sort((a, b) => {
        const valA = a[sortKey];
        const valB = b[sortKey];

        if (typeof valA === 'string' && typeof valB === 'string') {
            return sortAsc ? valA.localeCompare(valB) : valB.localeCompare(valA);
        } else if (typeof valA === 'number' && typeof valB === 'number') {
            return sortAsc ? valA - valB : valB - valA;
        }
        return 0;
    });

    // 3. Рендер
    tbody.innerHTML = '';
    
    // Оптимизация: используем DocumentFragment для пакетного добавления в DOM
    const fragment = document.createDocumentFragment();

    filteredProcs.forEach(proc => {
        const tr = document.createElement('tr');
        
        // Перевод страниц памяти в мегабайты
        const virtMB = ((proc.mem_size_pages * PAGE_SIZE_KB) / 1024).toFixed(1);
        const resMB = ((proc.mem_resident_pages * PAGE_SIZE_KB) / 1024).toFixed(1);

        tr.innerHTML = `
            <td>${proc.pid}</td>
            <td>${proc.owner}</td>
            <td class="proc-name">${proc.name}</td>
            <td>${proc.state}</td>
            <td>${proc.cpu_percent.toFixed(1)}</td>
            <td>${virtMB}M</td>
            <td>${resMB}M</td>
            <td class="proc-cmd" title="${proc.command}">${proc.command}</td>
        `;
        fragment.appendChild(tr);
    });

    tbody.appendChild(fragment);
}

// Утилита для безопасной установки текста
function setTextContent(id: string, text: string): void {
    const el = document.getElementById(id);
    if (el) el.textContent = text;
}