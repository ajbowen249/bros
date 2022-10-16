<template>
    <main>
        <button @click="() => $router.push({ name: 'explorer' })">Create New Disk</button>
        <hr />
        <button>Open .sav File</button>
    </main>
</template>

<script setup lang="ts">
import { ref, onMounted } from 'vue';

</script>
