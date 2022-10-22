<template>
    <main :class="[ editingEntry && 'half' ]">
        <div class="toolbar">
            <div>
                <button @click="showNewFileDialog = true">New File</button>
            </div>
            <div>
                <button @click="showNewFolderDialog = true">New Folder</button>
            </div>
            <div>
                <button :disabled="!clipboard" @click="paste">
                    Paste {{ clipboard?.name || '<none>' }}
                </button>
            </div>
            <div>
                <button @click="save">Save</button>
            </div>
            <div>
                <label for="entriesBar">Entries: {{ getEntries() }} / {{ MaxEntries }}&nbsp;</label>
                <progress id="entriesBar" :value="getEntries()" :max="MaxEntries" />
            </div>
            <div>
                <label for="blocksBar">Blocks: {{ getBlocks() }} / {{ MaxBlocks }}&nbsp;</label>
                <progress id="blocksBar" :value="getBlocks()" :max="MaxBlocks" />
            </div>
            <div>
                <label for="bytesBar">Bytes: {{ getBytes() }} / {{ MaxBytes }}&nbsp;</label>
                <progress id="bytesBar" :value="getBytes()" :max="MaxBytes" />
            </div>
        </div>
        <div>
            Folder:
            <span v-if="!selectedFolder">&lt;root&gt;</span>
            <span v-else><button @click="selectParent()">{{ selectedFolder.name.toString() }}</button></span>
        </div>
        <div v-for="(item, index) of getItems()" :key="index">
            <div v-if="item.entryType == EntryType.Folder">
                <button @click="select(item)">{{ item.name.toString() }}</button>
                &nbsp;<button @click="deleteItem(item)">&#x1F5D1;</button>
                <!-- IMPROVE: Folder movement -->
            </div>
            <div v-else-if="item.entryType == EntryType.File">
                {{ item.name.toString() }}
                &nbsp;<button @click="deleteItem(item)">&#x1F5D1;</button>
                &nbsp;<button @click="cutItem(item)">&#x2700;</button>
                &nbsp;<button @click="editFile(item)">&#x270F;</button>
            </div>
        </div>
        <div v-if="showNewFileDialog" class="dialog">
            <h1>
                New File
            </h1>

            <h3>
                Add from disk
            </h3>
            <input type="file" @change="addFileFromDisk" />

            <button @click="showNewFileDialog = false">Cancel</button>
        </div>
        <div v-if="showNewFolderDialog" class="dialog">
            <h1>
                New Folder
            </h1>

            <label for="newFolderNameInput">Name</label>
            <input v-model="newFolderNameInput" id="newFolderNameInput" type="text" />

            <button @click="createFolder">Create</button>
            <button @click="showNewFolderDialog = false">Cancel</button>
        </div>
    </main>
    <!-- eslint-disable-next-line vue/no-multiple-template-root -->
    <aside v-if="editingEntry" class="half editor-container">
        <div class="toolbar">
            <div>
                Editing {{ editingEntry.name.toString() }}
            </div>
            <div>
                <button @click="saveFile">Save</button>
            </div>
            <div>
                <button @click="editingEntry = null">Close</button>
            </div>
        </div>
        <textarea cols="32" rows="30" v-model="editingText" />
    </aside>
</template>

<script setup lang="ts">
import { ref, onMounted } from 'vue';
import * as bfs from '../BFSAbstract';
import * as bfsBinary from '../BFSBinary';
import { cloneDeep } from 'lodash';

const showNewFileDialog = ref(false);
const showNewFolderDialog = ref(false);

const newFolderNameInput = ref('');

const fileSystem = ref(bfs.createTestFileSystem());
const selectedFolder = ref<bfs.FolderEntry | null>(null);
const clipboard = ref<bfs.Entry | null>(null);
const editingEntry = ref<bfs.FileEntry | null>(null);
const editingText = ref<string>('');

const getEntries = () => fileSystem.value.getTable().length;
const getBlocks = () => fileSystem.value.getTable()
    .filter(x => x.entryType === bfs.EntryType.File)
    .reduce((acc, x) => {
        acc += Math.ceil((x as bfs.FileEntry)!.data.length / bfs.BlockDataSize);
        return acc;
    }, 0);

const getBytes = () => getBlocks() * bfs.BlockDataSize;

const MaxEntries = bfs.MaxEntries;
const MaxBlocks = bfs.MaxBlocks;
const MaxBytes = bfs.MaxBlocks * bfs.BlockDataSize;
const EntryType = bfs.EntryType;

function getItems(): bfs.Entry[] {
    return fileSystem.value.getTable().filter(x =>
        x.entryType !== bfs.EntryType.None &&
        x.parentFolder === selectedFolder.value
    );
}

function select(item: bfs.Entry) {
    if (item.entryType === bfs.EntryType.Folder) {
        selectedFolder.value = item as bfs.FolderEntry;
    }
}

function selectParent() {
    if (selectedFolder.value !== null) {
        selectedFolder.value = selectedFolder.value.parentFolder;
    }
}

function createName(inputName: string) {
    const nameParts = inputName.split('.');
    const name = nameParts[0].toUpperCase().slice(0, bfs.NameMaxLength);
    const extension = nameParts[1]?.toUpperCase()?.slice(0, bfs.MaxExtensionLength) ?? '';
    return new bfs.Name(name, extension);
}

async function addFileFromDisk(event: Event) {
    const target: HTMLInputElement = event.target as HTMLInputElement;
    if (!target || !target.files || target.files.length !== 1) {
        alert('Problem opening file');
        showNewFileDialog.value = false;
        return;
    }

    try {
        const file = target.files[0];
        const contentBuffer = await file.arrayBuffer();
        const fileContent = new Uint8Array(contentBuffer);

        const entry = new bfs.FileEntry(
            createName(file.name),
            selectedFolder.value as bfs.FolderEntry ?? undefined,
            fileContent
        );

        fileSystem.value.addEntry(entry, true);
    } catch {
        alert('Problem loading file');
    }

    showNewFileDialog.value = false;
}

function createFolder() {
    try {
        const entry = new bfs.FolderEntry(
            createName(newFolderNameInput.value),
            selectedFolder.value as bfs.FolderEntry ?? undefined
        );
        fileSystem.value.addEntry(entry, false);
    } catch {
        alert('Problem adding folder');
    }

    showNewFolderDialog.value = false;
}

function save() {
    const binary = bfsBinary.ToBinary(fileSystem.value as bfs.FileSystem);
    const file = new Blob([binary.buffer], { type: 'sav' });
    const anchorTag = document.createElement('a');
    const url = URL.createObjectURL(file);
    anchorTag.href = url;
    anchorTag.download = 'bros.sav';
    document.body.appendChild(anchorTag);
    anchorTag.click();
    setTimeout(function() {
        document.body.removeChild(anchorTag);
        window.URL.revokeObjectURL(url);
    }, 0); 
}

function deleteItem(item: bfs.Entry) {
    fileSystem.value.deleteEntry(item);
}

function cutItem(item: bfs.Entry) {
    clipboard.value = item;
    clipboard.value.parentFolder = null;
    fileSystem.value.deleteEntry(item);
}

function paste() {
    if (!clipboard.value) {
        return;
    }

    const entry = cloneDeep(clipboard.value);
    entry.parentFolder = selectedFolder.value;
    fileSystem.value.addEntry(entry as bfs.Entry, true);
}

function editFile(item: bfs.Entry) {
    if (item.entryType !== bfs.EntryType.File) {
        return;
    }

    editingEntry.value = item as bfs.FileEntry;
    const data = (item as bfs.FileEntry).data;
    editingText.value = new TextDecoder().decode(data);
}

function saveFile() {
    if (!editingEntry.value) {
        return;
    }

    editingEntry.value.data = new TextEncoder().encode(editingText.value.toUpperCase());
}

</script>

<style scoped lang="scss">
.toolbar {
    display: flex;
    flex-direction: row;
    flex-wrap: wrap;

    > div {
        margin-left: .2rem;
        margin-right: .2rem;
    }
}

.dialog {
    background-color: aqua;
    position: absolute;
    top: 0;
    left: 0;
    right: 0;
    bottom: 0;
    margin: 10rem;
    border-radius: 20px;

    padding: 1rem;
}

.half {
    max-width: 50vw;
}

.editor-container {
    position: absolute;
    left: 50vw;
    top: 0;
    bottom: 0;
    right: 0;
    border-left: 1px solid black;
}

</style>