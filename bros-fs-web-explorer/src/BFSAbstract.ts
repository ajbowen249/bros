export const Version = 1;
export const NameMaxLength = 12;
export const MaxExtensionLength = 4;
export const DefaultDiskVolumeSize = 4096;
export const HeaderSize = 8;
export const EntrySize = 20;
export const MaxEntries = 50;
export const BlockDataSize = 12;
export const BlockHeaderSize = 4;
export const NoFolder = 255;
export const BlockRecordSize = BlockDataSize + BlockHeaderSize;

const availableBlockSpace = DefaultDiskVolumeSize - HeaderSize - (MaxEntries * EntrySize);
export const MaxBlocks = availableBlockSpace / BlockRecordSize;
export const VolumeDataCapacity = MaxBlocks * BlockDataSize;

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

export class FileSystemOutOfSpaceError extends Error {
    constructor() {
        super('Out of disk space');
    }
}

export class Name {
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
        const limit = isName ? NameMaxLength : MaxExtensionLength;
        if (str.length > limit) {
            throw new (isName ? FileNameTooLongError : FileExtensionTooLongError)(str);
        }
    }
}

export enum EntryType {
    None = 0,
    Folder = 1,
    File = 2,
}

export class Entry {
    entryType: EntryType;
    parentFolder: FolderEntry | null;
    name: Name;

    constructor(
        entryType: EntryType = EntryType.None,
        name: Name = new Name('', ''),
        parentFolder?: FolderEntry,
    ) {
        this.entryType = entryType;
        this.name = name;
        this.parentFolder = parentFolder ?? null;
    }
}

export class FolderEntry extends Entry {
    placeholder: number;
    constructor(name: Name, parentFolder?: FolderEntry) {
        super(EntryType.Folder, name, parentFolder);
        this.placeholder = 0;
    }
}

export class FileEntry extends Entry {
    data: Uint8Array;
    constructor(name: Name, parentFolder?: FolderEntry, data?: Uint8Array) {
        super(EntryType.File, name, parentFolder);
        this.data = data ?? new Uint8Array([]);
    }
}

export class FileSystem {
    private table: Entry[];

    constructor() {
        this.table = [];
    }

    addEntry(entry: Entry) {
        if (this.table.length >= MaxEntries) {
            throw new FileSystemFullError();
        }

        this.table.push(entry);
    }

    getTable(): Entry[] {
        return this.table;
    }
}

export function createTestFileSystem(): FileSystem {
    const fs = new FileSystem();

    const sysFolder = new FolderEntry(new Name('SYS', ''));
    const emptySysFile = new FileEntry(new Name('EMPTYSYS', 'TXT'), sysFolder);
    const userFolder = new FolderEntry(new Name('USR', ''));
    const docsFolder = new FolderEntry(new Name('DOCUMENTS', ''), userFolder);
    const emptyDocFile = new FileEntry(new Name('EMPTYDOC', 'TXT'), docsFolder);
    const configFolder = new FolderEntry(new Name('CONFIG', ''), userFolder);
    const emptyRootFile = new FileEntry(new Name('EMPTYROOT', 'TXT'));

    fs.addEntry(sysFolder);
    fs.addEntry(emptySysFile);
    fs.addEntry(userFolder);
    fs.addEntry(docsFolder);
    fs.addEntry(emptyDocFile);
    fs.addEntry(configFolder);
    fs.addEntry(emptyRootFile);

    return fs;
}
