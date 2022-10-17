<template>
    <main>
        <div class="toolbar">
            <div>
                <button @click="showNewFileDialog = true">New File</button>
            </div>
            <div>
                <button @click="showNewFolderDialog = true">New Folder</button>
            </div>
        </div>
        <div>
            Folder:
            <span v-if="!selectedFolder">&lt;root&gt;</span>
            <span v-else><button @click="selectParent()">{{ selectedFolder.name.toString() }}</button></span>
        </div>
        <div v-for="(item, index) of getItems()" :key="index">
            <button @click="select(item)">{{ item.name.toString() }}</button>
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
</template>

<script setup lang="ts">
import { ref, onMounted } from 'vue';
import * as fs from '../BFSAbstract';

const showNewFileDialog = ref(false);
const showNewFolderDialog = ref(false);

const newFolderNameInput = ref('');

const fileSystem = ref(fs.createTestFileSystem());
const selectedFolder = ref<fs.FSFolderEntry | null>(null);

function getItems(): fs.FSEntry[] {
    return fileSystem.value.getTable().filter(x =>
        x.entryType !== fs.FSEntryType.None &&
        x.parentFolder === selectedFolder.value
    );
}

function select(item: fs.FSEntry) {
    if (item.entryType === fs.FSEntryType.Folder) {
        selectedFolder.value = item as fs.FSFolderEntry;
    }
}

function selectParent() {
    if (selectedFolder.value !== null) {
        selectedFolder.value = selectedFolder.value.parentFolder;
    }
}

function createFSName(inputName: string) {
    const nameParts = inputName.split('.');
    const name = nameParts[0].toUpperCase().slice(0, fs.BFSNameMaxLength);
    const extension = nameParts[1]?.toUpperCase()?.slice(0, fs.BFSMaxExtensionLength) ?? '';
    return new fs.FSName(name, extension);
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
        const fileContent = new Uint8ClampedArray(contentBuffer);

        const entry = new fs.FSFileEntry(
            createFSName(file.name),
            selectedFolder.value as fs.FSFolderEntry ?? undefined,
            fileContent
        );

        fileSystem.value.addEntry(entry);
    } catch {
        alert('Problem loading file');
    }

    showNewFileDialog.value = false;
}

function createFolder() {
    try {
        const entry = new fs.FSFolderEntry(
            createFSName(newFolderNameInput.value),
            selectedFolder.value as fs.FSFolderEntry ?? undefined
        );
        fileSystem.value.addEntry(entry);
    } catch {
        alert('Problem adding folder');
    }

    showNewFolderDialog.value = false;
}

</script>

<style scoped lang="scss">
.toolbar {
    display: flex;
    flex-direction: row;
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
</style>