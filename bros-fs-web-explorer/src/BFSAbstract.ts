export const BFSVersion = 1;
export const BFSNameMaxLength = 12;
export const BFSNameExtensionLength = 4;
export const BFSDefaultDiskVolumeSize = 4096;
export const BFSHeaderSize = 8;
export const BFSEntrySize = 20;
export const BFSMaxEntries = 50;
export const BFSBlockDataSize = 12;
export const BFSBlockHeaderSize = 4;
export const BFSBlockRecordSize = BFSBlockDataSize + BFSBlockHeaderSize;

export const BFSAvailableSpace = BFSDefaultDiskVolumeSize - BFSHeaderSize - (BFSMaxEntries * BFSEntrySize);
export const BFSMaxBlocks = BFSAvailableSpace / BFSBlockRecordSize;

export class FileNameTooLongError extends Error {
    public readonly offendingName: string;
    constructor(offendingName: string) {
        super(`File name too long (${offendingName})`);
        this.offendingName = offendingName;
    }
}

export class FileExtensionTooLongError extends Error {
    public readonly offendingExtension: string;
    constructor(offendingExtension: string) {
        super(`File extension too long (${offendingExtension})`);
        this.offendingExtension = offendingExtension;
    }
}

export class FileSystemFullError extends Error {
    constructor() {
        super('File table is full');
    }
}

export class FSName {
    private _name: string;
    private _extension: string;

    constructor(name: string, extension: string) {
        this.validateLength(name);
        this.validateLength(extension, false);
        this._name = name;
        this._extension = extension;
    }

    get name(): string { return this._name; }
    set name(newName: string) {
        this.validateLength(newName);
        this._name = newName;
    }

    get extension(): string { return this._extension; }
    set extension(newExtension: string) {
        this.validateLength(newExtension, false);
        this._extension = newExtension;
    }

    toString(): string {
        return `${this.name}${this.extension.length ? `.${this.extension}`  : ''}`;
    }

    private validateLength(str: string, isName: boolean = true) {
        const limit = isName ? BFSNameMaxLength : BFSNameExtensionLength;
        if (str.length > limit) {
            throw new (isName ? FileNameTooLongError : FileExtensionTooLongError)(str);
        }
    }
}

export enum FSEntryType {
    None = 0,
    Folder = 1,
    File = 2,
}

export class FSEntry {
    entryType: FSEntryType;
    parentFolder: FSFolderEntry | null;
    name: FSName;

    constructor(
        entryType: FSEntryType = FSEntryType.None,
        name: FSName = new FSName('', ''),
        parentFolder?: FSFolderEntry,
    ) {
        this.entryType = entryType;
        this.name = name;
        this.parentFolder = parentFolder ?? null;
    }
}

export class FSFolderEntry extends FSEntry {
    placeholder: number;
    constructor(name: FSName, parentFolder?: FSFolderEntry) {
        super(FSEntryType.Folder, name, parentFolder);
        this.placeholder = 0;
    }
}

export class FSFileEntry extends FSEntry {
    data: Uint8ClampedArray;
    constructor(name: FSName, parentFolder?: FSFolderEntry, data?: Uint8ClampedArray) {
        super(FSEntryType.File, name, parentFolder);
        this.data = data ?? new Uint8ClampedArray([]);
    }
}

export class FileSystem {
    private table: FSEntry[];

    constructor() {
        this.table = [];
    }

    addEntry(entry: FSEntry) {
        if (this.table.length >= BFSMaxEntries) {
            throw new FileSystemFullError();
        }

        this.table.push(entry);
    }

    getTable(): FSEntry[] {
        return this.table;
    }
}

export function createTestFileSystem(): FileSystem {
    const fs = new FileSystem();

    const sysFolder = new FSFolderEntry(new FSName('SYS', ''));
    const emptySysFile = new FSFileEntry(new FSName('EMPTYSYS', 'TXT'), sysFolder);
    const userFolder = new FSFolderEntry(new FSName('USR', ''));
    const docsFolder = new FSFolderEntry(new FSName('DOCUMENTS', ''), userFolder);
    const emptyDocFile = new FSFileEntry(new FSName('EMPTYDOC', 'TXT'), docsFolder);
    const configFolder = new FSFolderEntry(new FSName('CONFIG', ''), userFolder);
    const emptyRootFile = new FSFileEntry(new FSName('EMPTYROOT', 'TXT'));

    fs.addEntry(sysFolder);
    fs.addEntry(emptySysFile);
    fs.addEntry(userFolder);
    fs.addEntry(docsFolder);
    fs.addEntry(emptyDocFile);
    fs.addEntry(configFolder);
    fs.addEntry(emptyRootFile);

    return fs;
}
