import * as fs from './BFSAbstract';

function uint32ToBytes(num: number) {
    const bytes = new Uint32Array(num);
    return new Uint8ClampedArray(bytes);
}

function nameToBytes(name: fs.FSName) {
    const bytes = new Uint8ClampedArray(fs.BFSNameMaxLength + fs.BFSMaxExtensionLength);
    for (let i = 0; i < fs.BFSNameMaxLength; i++) {
        const char = i < name.name.length ? name.name.charCodeAt(i) : 0;
        bytes[i] = char;
    }

    for (let i = 0; i < fs.BFSMaxExtensionLength; i++) {
        const char = i < name.extension.length ? name.extension.charCodeAt(i) : 0;
        bytes[fs.BFSNameMaxLength + i] = char;
    }

    return bytes;
}

export function fsToBinary(fileSystem: fs.FileSystem) {
    const binary = new Uint8ClampedArray(fs.BFSDefaultDiskVolumeSize);
    const bfsString = 'BFS';
    binary[0] = bfsString.charCodeAt(0);
    binary[1] = bfsString.charCodeAt(1);
    binary[2] = bfsString.charCodeAt(2);
    binary[3] = fs.BFSVersion;

    const volumeBytes = uint32ToBytes(fs.BFSDefaultDiskVolumeSize);
    binary[4] = volumeBytes[0];
    binary[5] = volumeBytes[1];
    binary[6] = volumeBytes[2];
    binary[7] = volumeBytes[3];

    for (let i = 0; i < fs.BFSMaxEntries; i++) {
        const entry = fileSystem.getTable()[i] ?? new fs.FSEntry();
        const entryOffset = (i * fs.BFSEntrySize) + 8;
        binary[entryOffset] = entry.entryType;
        let folderIndex = entry.parentFolder ?
            fileSystem.getTable().indexOf(entry.parentFolder) :
            fs.BFSNoFolder;

        if (folderIndex < 0) {
            folderIndex = fs.BFSNoFolder;
        }

        binary[entryOffset + 1] = folderIndex;
        const nameBytes = nameToBytes(entry.name);
        // WHOA IT'S 10:00 GO TO BED!!!
    }
}
