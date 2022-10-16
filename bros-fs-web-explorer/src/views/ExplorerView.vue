<template>
    <main>
        <div>
            Folder:
            <span v-if="!selectedFolder">&lt;root&gt;</span>
            <span v-else><button @click="selectParent()">{{ selectedFolder.name.toString() }}</button></span>
        </div>
        <div v-for="(item, index) of getItems()" :key="index">
            <button @click="select(item)">{{ item.name.toString() }}</button>
        </div>
    </main>
</template>

<script setup lang="ts">
import { ref, onMounted } from 'vue';
import * as fs from '../BFSAbstract';

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

</script>
